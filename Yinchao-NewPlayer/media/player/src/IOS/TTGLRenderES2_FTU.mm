#import "TTGLRenderES2_FTU.h"
#import "TTLog.h"
#import "TTMacrodef.h"
//#import "voPackUV.h"

static const GLchar *G_VO_VERTEX_SHADER_Y_UV = " \
attribute vec4 position; \
attribute vec2 texCoord; \
varying vec2 texCoordVarying; \
void main() \
{ \
gl_Position = position; \
texCoordVarying = texCoord; \
}";


static const GLchar *G_VO_FRAGMENT_SHADER_Y_UV = " \
uniform sampler2D SamplerY; \
uniform sampler2D SamplerUV; \
varying highp vec2 texCoordVarying; \
void main() \
{ \
mediump vec3 yuv; \
lowp vec3 rgb; \
yuv.x = texture2D(SamplerY, texCoordVarying).r; \
yuv.yz = texture2D(SamplerUV, texCoordVarying).rg - vec2(0.5, 0.5); \
rgb = mat3(      1,       1,      1, \
0, -.18732, 1.8556, \
1.57481, -.46813,      0) * yuv; \
gl_FragColor = vec4(rgb, 1); \
}";

TTGLRenderES2_FTU::TTGLRenderES2_FTU(EAGLContext* pContext)
:TTGLRenderES2(pContext)
,_lumaTexture(NULL)
,_chromaTexture(NULL)
,videoTextureCache(NULL)
,m_nTextureUniformY(0)
,m_nTextureUniformUV(0)
{
}

TTGLRenderES2_FTU::~TTGLRenderES2_FTU()
{
    if (_lumaTexture)
    {
        CFRelease(_lumaTexture);
        _lumaTexture = NULL;
    }
    
    if (_chromaTexture)
    {
        CFRelease(_chromaTexture);
        _chromaTexture = NULL;
    }
    
    if (videoTextureCache) {
        CFRelease(videoTextureCache);
        videoTextureCache = 0;
    }
}

#pragma mark Protect function: GL setting.
int TTGLRenderES2_FTU::SetupFrameBuffer()
{
    int nRet = TTGLRenderES2::SetupFrameBuffer();

    if (TTKErrNone != nRet) {
        return nRet;
    }
    
    CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, m_pContext, NULL, &videoTextureCache);
    if (err) {
        LOGE("Error at CVOpenGLESTextureCacheCreate %d", err);
        return TTKErrArgument;
    }
    
    return TTKErrNone;
}

int TTGLRenderES2_FTU::DeleteFrameBuffer()
{
    // Periodic texture cache flush every frame
    if (videoTextureCache) {
        CFRelease(videoTextureCache);
        videoTextureCache = 0;
    }
    
    return TTGLRenderES2::DeleteFrameBuffer();
}

bool TTGLRenderES2_FTU::IsGLRenderReady() {
    
    if (!TTGLRenderES2::IsGLRenderReady()) {
        return false;
    }
    
    if (0 == videoTextureCache) {
        return false;
    }
    
    return true;
}

int TTGLRenderES2_FTU::TextureSizeChange()
{
    return TTKErrNone;
}

int TTGLRenderES2_FTU::SetupTexture()
{
    return TTKErrNone;
}

int TTGLRenderES2_FTU::DeleteTexture()
{
    return TTKErrNone;
}

int TTGLRenderES2_FTU::CompileAllShaders()
{
    if (m_nProgramHandle) {
        glDeleteProgram(m_nProgramHandle);
        m_nProgramHandle = 0;
    }
    
    // Setup program
    m_nProgramHandle = glCreateProgram();
    if (0 == m_nProgramHandle) {
        LOGE("CompileAllShaders fail m_nProgramHandle:%d", m_nProgramHandle);
        return TTKErrArgument;
    }
    
    GLuint vertexShader = CompileShader(G_VO_VERTEX_SHADER_Y_UV, GL_VERTEX_SHADER);
    GLuint fragmentShader = CompileShader(G_VO_FRAGMENT_SHADER_Y_UV, GL_FRAGMENT_SHADER);
    
    if (0 == vertexShader || 0 == fragmentShader) {
        LOGE("CompileAllShaders fail vertexShader:%d, fragmentShader:%d", vertexShader, fragmentShader);
        return TTKErrArgument;
    }
    
    glAttachShader(m_nProgramHandle, vertexShader);
    glAttachShader(m_nProgramHandle, fragmentShader);
    glLinkProgram(m_nProgramHandle);
    
    // Link program
    GLint linkSuccess;
    glGetProgramiv(m_nProgramHandle, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE) {
        
        if (vertexShader) {
            glDetachShader(m_nProgramHandle, vertexShader);
            glDeleteShader(vertexShader);
        }
        if (fragmentShader) {
            glDetachShader(m_nProgramHandle, fragmentShader);
            glDeleteShader(fragmentShader);
        }
        return TTKErrArgument;
    }
    
    // Use Program
    glUseProgram(m_nProgramHandle);
    
    // Get Attrib
    m_nPositionSlot = glGetAttribLocation(m_nProgramHandle, "position");
    glEnableVertexAttribArray(m_nPositionSlot);
    
    m_nTexCoordSlot = glGetAttribLocation(m_nProgramHandle, "texCoord");
    glEnableVertexAttribArray(m_nTexCoordSlot);
    
    m_nTextureUniformY = glGetUniformLocation(m_nProgramHandle, "SamplerY");
    m_nTextureUniformUV = glGetUniformLocation(m_nProgramHandle, "SamplerUV");
    
    // Release vertex and fragment shaders.
    if (vertexShader) {
        glDetachShader(m_nProgramHandle, vertexShader);
        glDeleteShader(vertexShader);
    }
    if (fragmentShader) {
        glDetachShader(m_nProgramHandle, fragmentShader);
        glDeleteShader(fragmentShader);
    }
    
    // Wrong init
    if (m_nTextureUniformY == m_nTextureUniformUV) {
        
        LOGE("Error Y:%d, UV:%d", m_nTextureUniformY, m_nTextureUniformUV);
        return TTKErrArgument;
    }
    
    LOGI("m_nPositionSlot:%d, m_nTexCoordSlot:%d, Y:%d, UV:%d", m_nPositionSlot, m_nTexCoordSlot, m_nTextureUniformY, m_nTextureUniformUV);
    
    return TTKErrNone;
}

#pragma mark Protect function: GL Draw
int TTGLRenderES2_FTU::UploadTexture(TTVideoBuffer *pVideoBuffer)
{
     CVImageBufferRef pixelBuffer = (CVImageBufferRef)pVideoBuffer->Buffer[0];
    if (NULL == pixelBuffer) {
        return TTKErrArgument;
    }
    
    if (_lumaTexture)
    {
        CFRelease(_lumaTexture);
        _lumaTexture = NULL;
    }
    
    if (_chromaTexture)
    {
        CFRelease(_chromaTexture);
        _chromaTexture = NULL;
    }
    
    // Periodic texture cache flush every frame
    CVOpenGLESTextureCacheFlush(videoTextureCache, 0);
    
    // Create a CVOpenGLESTexture from the CVImageBuffe
    
    CVReturn err = 0;
    glActiveTexture(GL_TEXTURE0);
    err = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                       videoTextureCache,
                                                       pixelBuffer,
                                                       NULL,
                                                       GL_TEXTURE_2D,
                                                       GL_RED_EXT,
                                                       m_nTextureWidth,
                                                       m_nTextureHeight,
                                                       GL_RED_EXT,
                                                       GL_UNSIGNED_BYTE,
                                                       0,
                                                       &_lumaTexture);
    if (err)
    {
        LOGE("Error at CVOpenGLESTextureCacheCreateTextureFromImage %d", err);
    }
    
    glBindTexture(CVOpenGLESTextureGetTarget(_lumaTexture), CVOpenGLESTextureGetName(_lumaTexture));
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // UV-plane
    glActiveTexture(GL_TEXTURE1);
    err = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                       videoTextureCache,
                                                       pixelBuffer,
                                                       NULL,
                                                       GL_TEXTURE_2D,
                                                       GL_RG_EXT,
                                                       m_nTextureWidth/2,
                                                       m_nTextureHeight/2,
                                                       GL_RG_EXT,
                                                       GL_UNSIGNED_BYTE,
                                                       1,
                                                       &_chromaTexture);
    if (err)
    {
        LOGE("Error at CVOpenGLESTextureCacheCreateTextureFromImage %d", err);
    }
    
    glBindTexture(CVOpenGLESTextureGetTarget(_chromaTexture), CVOpenGLESTextureGetName(_chromaTexture));
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    return TTKErrNone;
}

int TTGLRenderES2_FTU::RenderToScreen()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Update attribute values.
	glVertexAttribPointer(m_nPositionSlot, 2, GL_FLOAT, 0, 0, m_fSquareVertices);
	glVertexAttribPointer(m_nTexCoordSlot, 2, GL_FLOAT, 0, 0, m_fTextureVertices);
    
    glUniform1i(m_nTextureUniformY, 0);
    glUniform1i(m_nTextureUniformUV, 1);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    return TTKErrNone;
}

int TTGLRenderES2_FTU::RedrawInner()
{
    TTCAutoLock cAuto(&mCritical);
    
    if (!IsGLRenderReady()) {
        LOGW("RenderYUV not ready");
        return TTKErrNotReady;
    }
    
    /*if (NULL == pixelBuffer) {
        return TT_GL_RET_ERR_STATUS;
    }*/
    
    if (![EAGLContext setCurrentContext:m_pContext]) {
        return TTKErrArgument;
    }
    
    RenderToScreen();
    
    //GetLastFrameInner(bIsTryGetFrame, pData);
    
    RenderCommit();

    return TTKErrNone;
}
