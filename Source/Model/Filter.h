/*
 Copyright (C) 2010-2012 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_Filter_h
#define TrenchBroom_Filter_h

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/Face.h"
#include "Utility/String.h"
#include "View/ViewOptions.h"

namespace TrenchBroom {
    namespace Model {
        class Filter {
        public:
            virtual ~Filter() {}

            virtual bool entityVisible(const Model::Entity& entity) const = 0;
            virtual bool entityPickable(const Model::Entity& entity) const = 0;
            virtual bool brushVisible(const Model::Brush& brush) const = 0;
            virtual bool brushPickable(const Model::Brush& brush) const = 0;
            virtual bool brushVerticesPickable(const Model::Brush& brush) const = 0;
        };

        class DefaultFilter : public Filter {
        protected:
            const View::ViewOptions& m_viewOptions;
        public:
            DefaultFilter(const View::ViewOptions& viewOptions) :
            m_viewOptions(viewOptions) {}

            virtual inline bool entityVisible(const Model::Entity& entity) const {
                EntityDefinition* definition = entity.definition();
                if (definition != NULL && definition->type() == EntityDefinition::PointEntity && m_viewOptions.showEntities() == false)
                    return false;

                if (entity.hidden() || entity.worldspawn())
                    return false;

                const String& pattern = m_viewOptions.filterPattern();
                if (!pattern.empty()) {
                    const Model::Properties& properties = entity.properties();
                    Model::Properties::const_iterator it, end;
                    for (it = properties.begin(), end = properties.end(); it != end; ++it) {
                        const PropertyKey& key = it->first;
                        const PropertyValue& value = it->second;
                        if (Utility::containsString(key, pattern, false) ||
                            Utility::containsString(value, pattern, false))
                            return true;
                    }
                    return false;
                }

                return true;
            }

            virtual inline bool entityPickable(const Model::Entity& entity) const {
                if (entity.worldspawn() ||
                    entity.locked() ||
                    !entity.brushes().empty() ||
                    !entityVisible(entity))
                    return false;

                return true;
            }

            virtual inline bool brushVisible(const Model::Brush& brush) const {
                if (!m_viewOptions.showBrushes() || brush.hidden())
                    return false;

                const String& pattern = m_viewOptions.filterPattern();
                if (!pattern.empty() || !m_viewOptions.showClipBrushes() || !m_viewOptions.showSkipBrushes()) {
                    const Model::FaceList& faces = brush.faces();
                    unsigned int clipCount = 0;
                    unsigned int skipCount = 0;
                    bool matches = pattern.empty();
                    for (unsigned int i = 0; i < faces.size(); i++) {
                        const String& textureName = faces[i]->textureName();
                        if (!m_viewOptions.showClipBrushes() && Utility::containsString(textureName, "clip", false))
                            clipCount++;
                        if (!m_viewOptions.showSkipBrushes() && Utility::containsString(textureName, "skip", false))
                            skipCount++;
                        if (!matches)
                            matches = Utility::containsString(textureName, pattern, false);
                    }

                    if (!m_viewOptions.showClipBrushes() && clipCount == faces.size())
                        return false;
                    if (!m_viewOptions.showSkipBrushes() && skipCount == faces.size())
                        return false;

                    return matches;
                }

                return true;
            }

            virtual inline bool brushPickable(const Model::Brush& brush) const {
                if (!brushVisible(brush) || brush.locked() || brush.entity()->locked())
                    return false;

                return true;
            }

            virtual inline bool brushVerticesPickable(const Model::Brush& brush) const {
                if (!brushPickable(brush))
                    return false;

                return true;
            }
        };
    }
}

#endif
