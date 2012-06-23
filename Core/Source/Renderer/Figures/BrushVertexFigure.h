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

#ifndef TrenchBroom_BrushVertexFigure_h
#define TrenchBroom_BrushVertexFigure_h

#include "Renderer/Figures/Figure.h"
#include "Model/Map/BrushTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        
        class BrushVertexFigure : public Figure {
        protected:
            Vbo* m_vbo;
            bool m_valid;
            unsigned int m_vertexCount;
            Model::BrushList m_brushes;
        public:
            BrushVertexFigure() : m_vbo(NULL), m_valid(false) {}
            ~BrushVertexFigure();
            
            void setBrushes(const Model::BrushList& brushes);
            virtual void render(RenderContext& context);
        };
    }
}

#endif
