#define STB_IMAGE_IMPLEMENTATION
#include "OpenGLTexture2D.h"

#include <stb_image.h>
#include <spdlog/spdlog.h>

namespace Scythe
{
    OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
        : m_Path(path)
    {
        int width, height, channels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        
        if (!data) 
        {
            spdlog::error("Failed to load texture: {}", path);
            m_Width = 0; m_Height = 0; m_RendererID = 0;
            return;
        }

        m_Width = width;
        m_Height = height;
        
        GLenum internalFormat = 0, dataFormat = 0;
        if (channels == 4) 
        {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        } 
        else if (channels == 3) 
        {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        } 
        else if (channels == 1) 
        {
            internalFormat = GL_R8;
            dataFormat = GL_RED;
        }
        
        m_InternalFormat = internalFormat;
        m_DataFormat = dataFormat;

        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);

        glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        glDeleteTextures(1, &m_RendererID);
    }
    
    void OpenGLTexture2D::Bind(uint32_t slot) const 
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
    }

    void OpenGLTexture2D::Unbind() const 
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
