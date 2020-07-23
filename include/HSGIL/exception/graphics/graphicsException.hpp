/********************************************************************************
 *                                                                              *
 * HSGIL - Handy Scalable Graphics Integration Library                          *
 * Copyright (c) 2020 Adrian Bedregal and Gabriela Chipana                      *
 *                                                                              *
 * This software is provided 'as-is', without any express or implied            *
 * warranty. In no event will the authors be held liable for any damages        *
 * arising from the use of this software.                                       *
 *                                                                              *
 * Permission is granted to anyone to use this software for any purpose,        *
 * including commercial applications, and to alter it and redistribute it       *
 * freely, subject to the following restrictions:                               *
 *                                                                              *
 * 1. The origin of this software must not be misrepresented; you must not      *
 *    claim that you wrote the original software. If you use this software      *
 *    in a product, an acknowledgment in the product documentation would be     *
 *    appreciated but is not required.                                          *
 * 2. Altered source versions must be plainly marked as such, and must not be   *
 *    misrepresented as being the original software.                            *
 * 3. This notice may not be removed or altered from any source distribution.   *
 *                                                                              *
 ********************************************************************************/

#ifndef HSGIL_GRAPHICS_EXCEPTION_HPP
#define HSGIL_GRAPHICS_EXCEPTION_HPP

#include <HSGIL/exception/genericException.hpp>

namespace gil
{
/**
 * @brief Graphics Generic Exception
 * 
 */
class GraphicsException : public GenericException
{
public:
    /**
     * @brief Overridden method to know why exactly the Graphics Exception was thrown
     * 
     * @return const char* 
     */
    virtual const char* what() const throw() override;
};

/**
 * @brief Shader Exception thrown when a Shader fails when creating
 * 
 */
class ShaderException : public GraphicsException
{
public:
    /**
     * @brief Overridden method to know why exactly the Shader Exception was thrown
     * 
     * @return const char* 
     */
    virtual const char* what() const throw() override;
};

#include <HSGIL/exception/graphics/graphicsException.inl>

} // namespace gil

#endif // HSGIL_GRAPHICS_EXCEPTION_HPP
