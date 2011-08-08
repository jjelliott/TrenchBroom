//
//  ClipPlane.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ClipPlane.h"
#import "Map.h"
#import "Entity.h"
#import "Face.h"
#import "MutableFace.h"
#import "Brush.h"
#import "MutableBrush.h"
#import "PickingHit.h"
#import "PickingHitList.h"

@implementation ClipPlane

- (void)addPoint:(TVector3i *)thePoint hitList:(PickingHitList *)theHitList {
    NSAssert(numPoints >= 0 && numPoints < 3, @"number of points must be between 0 and 3");
    
    points[numPoints] = *thePoint;
    hitLists[numPoints] = [theHitList retain];
    numPoints++;
}

- (void)updatePoint:(int)index x:(int)x y:(int)y z:(int)z {
    NSAssert(index >= 0 && index < numPoints, @"index out of bounds");
    
    points[index].x = x;
    points[index].y = y;
    points[index].z = z;
}

- (void)removeLastPoint {
    NSAssert(numPoints > 0 && numPoints <= 3, @"number of points must be between 0 and 3");

    numPoints--;
    [hitLists[numPoints] release];
    hitLists[numPoints] = nil;
}

- (int)numPoints {
    return numPoints;
}

- (TVector3i *)point:(int)index {
    NSAssert(index >= 0 && index <= numPoints, @"index out of bounds");
    
    return &points[index];
}

- (PickingHitList *)hitList:(int)index {
    NSAssert(index >= 0 && index <= numPoints, @"index out of bounds");
    
    return hitLists[index];
}

- (void)setClipMode:(EClipMode)theClipMode {
    clipMode = theClipMode;
}

- (EClipMode)clipMode {
    return clipMode;
}

- (MutableFace *)face:(BOOL)front {
    if (numPoints < 2)
        return nil;

    TVector3i* p1 = [self point:0];
    TVector3i* p2 = [self point:1];
    TVector3i* p3 = NULL;
    
    if (numPoints < 3) {
        const TVector3f* norm = NULL;
        if (p1->x != p2->x && 
            p1->y != p2->y && 
            p1->z != p2->z) {
            norm = &ZAxisPos;
        } else if (p1->x == p2->x && 
                   p1->y != p2->y && 
                   p1->z != p2->z) {
            norm = &XAxisPos;
        } else if (p1->x != p2->x && 
                   p1->y == p2->y && 
                   p1->z != p2->z) {
            norm = &YAxisPos;
        } else if (p1->x != p2->x && 
                   p1->y != p2->y && 
                   p1->z == p2->z) {
            norm = &ZAxisPos;
        } else {
            NSSet* faces1 = [[self hitList:0] objectsOfType:HT_FACE];
            NSSet* faces2 = [[self hitList:1] objectsOfType:HT_FACE];
            
            NSSet* both = [[NSMutableSet alloc] initWithSet:faces1];
            [both intersectsSet:faces2];
            
            if ([both count] > 0) {
                id <Face> face = [[both objectEnumerator] nextObject];
                norm = [face norm];
            }
            
            [both release];
        }
        
        if (norm != NULL) {
            TVector3f t;
            scaleV3f(norm, -10000, &t);
            
            p3 = malloc(sizeof(TVector3f));
            p3->x = roundf(t.x);
            p3->y = roundf(t.y);
            p3->z = roundf(t.z);
            addV3i(p3, p1, p3);
        }    
    } else {
        p3 = [self point:2];
    }
    
    if (p3 == NULL)
        return nil;
    
    id <Face> template = [[[self hitList:0] firstHitOfType:HT_FACE ignoreOccluders:YES] object];
    
    MutableFace* face = nil;
    if (front)
        face = [[MutableFace alloc] initWithPoint1:p1 point2:p2 point3:p3 texture:[template texture]];
    else
        face = [[MutableFace alloc] initWithPoint1:p3 point2:p2 point3:p1 texture:[template texture]];

    [face setXOffset:[template xOffset]];
    [face setYOffset:[template yOffset]];
    [face setXScale:[template xScale]];
    [face setYScale:[template yScale]];
    [face setRotation:[template rotation]];
    
    if (numPoints < 3)
        free(p3);
    
    return [face autorelease];
}

- (void)clipBrush:(id <Brush>)brush firstResult:(id <Brush>*)firstResult secondResult:(id <Brush>*)secondResult {
    MutableFace* clipFace = clipMode == CM_BACK ? [self face:NO] : [self face:YES];
    if (clipFace != nil) {
        id <Map> map = [[brush entity] map];
        MutableBrush* newBrush = [[MutableBrush alloc] initWithWorldBounds:[map worldBounds] brushTemplate:brush];

        if (![newBrush addFace:clipFace]) {
            [newBrush release];
            newBrush = nil;
        } else {
            *firstResult = newBrush;
            [newBrush autorelease];
        }
        
        if (clipMode == CM_SPLIT) {
            clipFace = [self face:NO];
            if (clipFace != nil) {
                newBrush = [[MutableBrush alloc] initWithWorldBounds:[map worldBounds] brushTemplate:brush];
                if (![newBrush addFace:clipFace]) {
                    [newBrush release];
                    newBrush = nil;
                } else {
                    *secondResult = newBrush;
                    [newBrush autorelease];
                }
            }
        }
    }
}

- (void)reset {
    for (int i = 0; i < numPoints; i++) {
        [hitLists[i] release];
        hitLists[i] = nil;
    }
    
    numPoints = 0;
}

- (void)dealloc {
    [self reset];
    [super dealloc];
}

@end
