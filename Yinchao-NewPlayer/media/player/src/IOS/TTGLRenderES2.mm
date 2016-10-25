#import "TTGLRenderES2.h"
#import "TTMacrodef.h"
#import "TTLog.h"

static const GLchar *G_VO_VERTEX_SHADER_Y_U_V = " \
attribute vec4 position; \
attribute mediump vec4 textureCoordinate; \
varying mediump vec2 coordinate; \
void main() \
{ \
gl_Position = position; \
coordinate = textureCoordinate.xy; \
}";


static const GLchar *G_VO_FRAGMENT_SHADER_Y_U_V = " \
precision highp float; \
uniform sampler2D SamplerY; \
uniform sampler2D SamplerU; \
uniform sampler2D SamplerV; \
\
varying highp vec2 coordinate; \
\
void main() \
{ \
highp vec3 yuv,yuv1; \
highp vec3 rgb; \
\
yuv.x = texture2D(SamplerY, coordinate).r; \
\
yuv.y = texture2D(SamplerU, coordinate).r-0.5; \
\
yuv.z = texture2D(SamplerV, coordinate).r-0.5 ; \
\
rgb = mat3(      1,       1,      1, \
0, -.34414, 1.772, \
1.402, -.71414,      0) * yuv; \
\
gl_FragColor = vec4(rgb, 1); \
}";

TTGLRenderES2::TTGLRenderES2(EAGLContext* pContext)
:TTGLRenderBase(pContext)
,m_pFrameData(NULL)
,m_pLastGetFrame(NULL)
,m_fOutLeftPer(0)
,m_fOutTopPer(0)
,m_fOutRightPer(0)
,m_fOutBottomPer(0)
,m_nPositionSlot(0)
,m_nTexCoordSlot(0)
,m_nColorRenderBuffer(0)
,m_nFrameBuffer(0)
,m_nProgramHandle(0)
,m_nTexturePlanarY(0)
,m_nTexturePlanarU(0)
,m_nTexturePlanarV(0)
,m_nTextureUniformY(0)
,m_nTextureUniformU(0)
,m_nTextureUniformV(0)
,m_bInitSuccess(false)
,frame_draw_x(0)
,frame_draw_y(0)
,frame_draw_h(0)
,frame_draw_w(0)
{
    m_fTextureVertices[0] = 0;
    m_fTextureVertices[1] = 1;
    
    m_fTextureVertices[2] = 1;
    m_fTextureVertices[3] = 1;
    
    m_fTextureVertices[4] = 0;
    m_fTextureVertices[5] = 0;
    
    m_fTextureVertices[6] = 1;
    m_fTextureVertices[7] = 0;
    
    memset(&m_fSquareVertices, 0, sizeof(m_fSquareVertices));
    
    [EAGLContext setCurrentContext:m_pContext];
}

int TTGLRenderES2::init()
{
    TTCAutoLock Lock(&mCritical);
    
    if (m_bInitSuccess) {
        return TTKErrNone;
    }
    
    int nRet = SetupTexture();
    if (TTKErrNone != nRet) {
        return nRet;
    }
    
    nRet = CompileAllShaders();
    if (TTKErrNone != nRet) {
        return nRet;
    }
    
    UpdateVertices();
    
    LOGI("init success:%p", this);
    
    m_bInitSuccess = true;
    
    return nRet;
}

bool TTGLRenderES2::IsGLRenderReady() {
    
    if (!m_bInitSuccess) {
        return false;
    }
    
    if (0 == m_nColorRenderBuffer)
	{
		return false;
	}
    
    if (0 == m_nFrameBuffer)
	{
        return false;
    }
    
    return true;
}

TTGLRenderES2::~TTGLRenderES2()
{
    TTCAutoLock Lock(&mCritical);
    
    LOGI("Delete TTGLRenderES2:%p", this);
    
    DeleteRenderBuffer();
    DeleteFrameBuffer();
    DeleteTexture();
    
    if (m_nProgramHandle) {
        glDeleteProgram(m_nProgramHandle);
        m_nProgramHandle = 0;
    }
    
    if (NULL != m_pFrameData) {
        delete []m_pFrameData;
        m_pFrameData = NULL;
    }
    
    if (NULL != m_pLastGetFrame) {
        free(m_pLastGetFrame);
        m_pLastGetFrame = NULL;
    }
}

#pragma mark Public function.

int TTGLRenderES2::SetTexture(int nWidth, int nHeight)
{
    TTCAutoLock Lock(&mCritical);
    
    if (NULL == m_pContext || ![EAGLContext setCurrentContext:m_pContext]) {
        return TTKErrArgument;
    }
    
    if ((nWidth != m_nTextureWidth) || (nHeight != m_nTextureHeight)) {
        m_nTextureWidth = nWidth;
        m_nTextureHeight = nHeight;
        TextureSizeChange();
        
        UpdateVertices();
    }
    
    return TTKErrNone;
}

int TTGLRenderES2::TextureSizeChange()
{
    if (NULL != m_pFrameData) {
        delete []m_pFrameData;
        m_pFrameData = NULL;
    }
    
    return TTKErrNone;
}

int TTGLRenderES2::ClearGL()
{
    TTCAutoLock Lock(&mCritical);
    
    if (NULL != m_pFrameData) {
        delete []m_pFrameData;
        m_pFrameData = NULL;
    }
    
    if (NULL == m_pContext || ![EAGLContext setCurrentContext:m_pContext]) {
        return TTKErrArgument;
    }
    
    glClearColor(0.0, 0.0, 0.0, 1.0);
    
    /* Clear the renderbuffer */
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
	
	/* Refresh the screen */
	[m_pContext presentRenderbuffer: GL_RENDERBUFFER];
    
    return TTKErrNone;
}

int TTGLRenderES2::Redraw()
{
    return RedrawInner(false, NULL);
}

int TTGLRenderES2::GetLastFrame(void *pData)//void
{
    TTCAutoLock Lock(&mCritical);
    
    Swap(&(m_fSquareVertices[0]), &(m_fSquareVertices[4]));
    Swap(&(m_fSquareVertices[1]), &(m_fSquareVertices[5]));
    Swap(&(m_fSquareVertices[2]), &(m_fSquareVertices[6]));
    Swap(&(m_fSquareVertices[3]), &(m_fSquareVertices[7]));

    int nRet = RedrawInner(true, pData);
    
    Swap(&(m_fSquareVertices[0]), &(m_fSquareVertices[4]));
    Swap(&(m_fSquareVertices[1]), &(m_fSquareVertices[5]));
    Swap(&(m_fSquareVertices[2]), &(m_fSquareVertices[6]));
    Swap(&(m_fSquareVertices[3]), &(m_fSquareVertices[7]));
    
    return nRet;
}

int TTGLRenderES2::RenderYUV(TTVideoBuffer *pVideoBuffer)
{
    CGRect cFrame = [m_pUIViewSet frame];
    if ( IsEqual(m_cFrame.size.width, cFrame.size.width)
        && IsEqual(m_cFrame.size.height, cFrame.size.height)) {
    }
    else{
        m_cFrame = [m_pUIViewSet frame];
        RefreshView();
    }
    
    TTCAutoLock Lock(&mCritical);
    
    if (!IsGLRenderReady()) {
        LOGE("RenderYUV not ready");
        return TTKErrNotReady;
    }
    
    if (![EAGLContext setCurrentContext:m_pContext]) {
        return TTKErrArgument;
    }
    
    int nRet = UploadTexture(pVideoBuffer);
    if (TTKErrNone != nRet) {
        return nRet;
    }
    
    RenderToScreen();
    RenderCommit();
    
    return TTKErrNone;
}

int TTGLRenderES2::GetSupportType()
{
    return TT_GL_SUPPORT_Y_U_V;
}

#pragma mark Protect function: GL setting.
int TTGLRenderES2::RefreshView()
{
    TTCAutoLock Lock(&mCritical);
    
    if (!m_pVideoView) {
        return TTKErrArgument;
    }
    
    if (NULL == m_pContext || ![EAGLContext setCurrentContext:m_pContext]) {
        return TTKErrArgument;
    }
    
    CAEAGLLayer* _eaglLayer = (CAEAGLLayer *)m_pVideoView.layer;
    _eaglLayer.opaque = YES;
    
    SetupRenderBuffer();
    SetupFrameBuffer();
    
    SetupGlViewport();
    
    UpdateVertices();
    
    return TTKErrNone;
}

int TTGLRenderES2::SetupGlViewport()
{
    /* Extract renderbuffer's width and height. This should match layer's size, I assume */
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &m_nBackingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &m_nBackingHeight);
	
	/* Set viewport size to match the renderbuffer size */
	glViewport(0, 0, m_nBackingWidth, m_nBackingHeight);
    
    glClearColor(0.0, 0.0, 0.0, 1.0);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    return TTKErrNone;
}

int TTGLRenderES2::SetupRenderBuffer()
{
    DeleteRenderBuffer();
    glGenRenderbuffers(1, &m_nColorRenderBuffer);
    
    if (0 == m_nColorRenderBuffer)
	{
        LOGE("SetupRenderBuffer fail");
		return TTKErrArgument;
	}
    
    glBindRenderbuffer(GL_RENDERBUFFER, m_nColorRenderBuffer);
    [m_pContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)m_pVideoView.layer];
    
    return TTKErrNone;
}

int TTGLRenderES2::SetupFrameBuffer()
{
    DeleteFrameBuffer();
    glGenFramebuffers(1, &m_nFrameBuffer);
    
    if (0 == m_nFrameBuffer) {
        LOGE("SetupFrameBuffer fail");
        return TTKErrArgument;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_nFrameBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER, m_nColorRenderBuffer);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGE("Failure with framebuffer generation");
		return TTKErrArgument;
	}
    
    return TTKErrNone;
}

int TTGLRenderES2::SetupTexture()
{
    DeleteTexture();
    glGenTextures(1, &m_nTexturePlanarY);
    glGenTextures(1, &m_nTexturePlanarU);
    glGenTextures(1, &m_nTexturePlanarV);
    
    if ((0 == m_nTexturePlanarY) || (0 == m_nTexturePlanarU) || (0 == m_nTexturePlanarV)) {
        LOGE("SetupTexture fail, %d, %d, %d", m_nTexturePlanarY, m_nTexturePlanarU, m_nTexturePlanarV);
        return TTKErrArgument;
    }
    
    return TTKErrNone;
}

int TTGLRenderES2::DeleteRenderBuffer()
{
    if (m_nColorRenderBuffer)
	{
		glDeleteRenderbuffers(1, &m_nColorRenderBuffer);
		m_nColorRenderBuffer = 0;
	}
    return TTKErrNone;
}

int TTGLRenderES2::DeleteFrameBuffer()
{
	if (m_nFrameBuffer)
	{
		glDeleteFramebuffers(1, &m_nFrameBuffer);
		m_nFrameBuffer = 0;
	}
    
    return TTKErrNone;
}

int TTGLRenderES2::DeleteTexture()
{
    if (m_nTexturePlanarY)
	{
		glDeleteTextures(1, &m_nTexturePlanarY);
		m_nTexturePlanarY = 0;
	}
    
    if (m_nTexturePlanarU)
	{
		glDeleteTextures(1, &m_nTexturePlanarU);
		m_nTexturePlanarU = 0;
	}
    
    if (m_nTexturePlanarV)
	{
		glDeleteTextures(1, &m_nTexturePlanarV);
		m_nTexturePlanarV = 0;
	}
    
    return TTKErrNone;
}

GLuint TTGLRenderES2::CompileShader(const GLchar *pBuffer, GLenum shaderType)
{
    NSString* shaderString = [NSString stringWithFormat:@"%s", pBuffer];

    // 2
    GLuint shaderHandle = glCreateShader(shaderType);
    
    // 3
    const char* shaderStringUTF8 = [shaderString UTF8String];
    int shaderStringLength = [shaderString length];
    glShaderSource(shaderHandle, 1, &shaderStringUTF8, &shaderStringLength);
    
    // 4
    glCompileShader(shaderHandle);
    
    // 5
    GLint compileSuccess;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);
    if (compileSuccess == GL_FALSE) {
#ifdef _VOLOG_ERROR
        GLchar messages[256];
        glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);
        NSString *messageString = [NSString stringWithUTF8String:messages];
        LOGE("%s", [messageString UTF8String]);
#endif
        return -1;
    }
    
    return shaderHandle;
}

int TTGLRenderES2::CompileAllShaders()
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
    
    GLuint vertexShader = CompileShader(G_VO_VERTEX_SHADER_Y_U_V, GL_VERTEX_SHADER);
    GLuint fragmentShader = CompileShader(G_VO_FRAGMENT_SHADER_Y_U_V, GL_FRAGMENT_SHADER);
    
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
        
#ifdef _VOLOG_ERROR
        GLchar messages[256];
        glGetProgramInfoLog(m_nProgramHandle, sizeof(messages), 0, &messages[0]);
        NSString *messageString = [NSString stringWithUTF8String:messages];
        LOGE("%s", [messageString UTF8String]);
#endif
        return TTKErrArgument;
    }
    
    // Use Program
    glUseProgram(m_nProgramHandle);
    
    // Get Attrib
    m_nPositionSlot = glGetAttribLocation(m_nProgramHandle, "position");
    //启用与m_nPositionSlot索引相关联的顶点数组
    glEnableVertexAttribArray(m_nPositionSlot);
    
    m_nTexCoordSlot = glGetAttribLocation(m_nProgramHandle, "textureCoordinate");
    glEnableVertexAttribArray(m_nTexCoordSlot);
    
    m_nTextureUniformY = glGetUniformLocation(m_nProgramHandle, "SamplerY");
    m_nTextureUniformU = glGetUniformLocation(m_nProgramHandle, "SamplerU");
    m_nTextureUniformV = glGetUniformLocation(m_nProgramHandle, "SamplerV");
    
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
    if ((m_nTextureUniformY == m_nTextureUniformU) || (m_nTextureUniformU == m_nTextureUniformV) || (m_nTextureUniformY == m_nTextureUniformV)) {
        
        LOGE("Error Y:%d, U:%d, V:%d", m_nTextureUniformY, m_nTextureUniformU, m_nTextureUniformV);
        return TTKErrArgument;
    }
    
    LOGI("m_nPositionSlot:%d, m_nTexCoordSlot:%d, Y:%d, U:%d, V:%d", m_nPositionSlot, m_nTexCoordSlot, m_nTextureUniformY, m_nTextureUniformU, m_nTextureUniformV);
    
    return TTKErrNone;
}

#pragma mark Protect function: GL Draw.
int TTGLRenderES2::GLTexImage2D(GLuint nTexture, Byte *pDate, int nWidth, int nHeight)
{
    glBindTexture(GL_TEXTURE_2D, nTexture);
    
    //如果图元的大小不等于纹理的大小，OpenGL便会对纹理进行缩放以适应图元的尺寸,GL_LINEAR:线性内部插值
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    //设置缠绕方式 -->s  ,t为向下方向
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, nWidth, nHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pDate);
    
    return TTKErrNone;
}

int TTGLRenderES2::UploadTexture(TTVideoBuffer *pVideoBuffer)
{
    if (NULL == m_pFrameData) {
        m_pFrameData = new unsigned char[m_nTextureWidth * m_nTextureHeight * 3 / 2];
    }
    
    if (NULL == m_pFrameData) {
        return TTKErrArgument;
    }
    
    int i = 0;
    int nWidthUV = m_nTextureWidth / 2;
    int nHeightUV = m_nTextureHeight / 2;
    
    for (i = 0; i < m_nTextureHeight; i++)
        memcpy (m_pFrameData + m_nTextureWidth * i, pVideoBuffer->Buffer[0] + pVideoBuffer->Stride[0] * i, m_nTextureWidth);
    
    unsigned char* pBuffer = m_pFrameData + (m_nTextureWidth * m_nTextureHeight);
    for (i = 0; i < nHeightUV; i++)
        memcpy (pBuffer + (nWidthUV * i), pVideoBuffer->Buffer[1] + pVideoBuffer->Stride[1] * i, nWidthUV);
    
    pBuffer = pBuffer + (nWidthUV * nHeightUV);
    for (i = 0; i < nHeightUV; i++)
        memcpy (pBuffer + (nWidthUV * i), pVideoBuffer->Buffer[2] + pVideoBuffer->Stride[2] * i, nWidthUV);
    
    GLTexImage2D(m_nTexturePlanarY, m_pFrameData, m_nTextureWidth, m_nTextureHeight);
    GLTexImage2D(m_nTexturePlanarU, pBuffer - (nWidthUV * nHeightUV), nWidthUV, nHeightUV);
    GLTexImage2D(m_nTexturePlanarV, pBuffer, nWidthUV, nHeightUV);
    
    return TTKErrNone;
}

int TTGLRenderES2::RenderToScreen()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    
    glClear(GL_COLOR_BUFFER_BIT);//清除颜色缓存数据
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_nTexturePlanarY);
    glUniform1i(m_nTextureUniformY, 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_nTexturePlanarU);
    glUniform1i(m_nTextureUniformU, 1);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_nTexturePlanarV);
    glUniform1i(m_nTextureUniformV, 2);
    
    // Update attribute values.为顶点着色器位置信息赋值,2表示每一个顶点信息由几个值组成,即将m_fSquareVertices传入顶点着色器中m_nPositionSlot对应的数组中去。stride=0表示数据紧挨着放
	glVertexAttribPointer(m_nPositionSlot, 2, GL_FLOAT, 0, 0, m_fSquareVertices);
	glVertexAttribPointer(m_nTexCoordSlot, 2, GL_FLOAT, 0, 0, m_fTextureVertices);
    
    //将顶点数组使用三角形渲染,0表示数组第一个值的位置，4表示数组长度，GL_TRIANGLE_STRIP可render成矩形
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    return TTKErrNone;
}

int TTGLRenderES2::RenderCommit()
{
    [m_pContext presentRenderbuffer:GL_RENDERBUFFER];
    return TTKErrNone;
}

#pragma mark Protect function: Other.
int TTGLRenderES2::RedrawInner(bool bIsTryGetFrame, void *pData)
{
    TTCAutoLock Lock(&mCritical);
    
    if (!IsGLRenderReady()) {
        LOGE("RenderYUV not ready");
        return TTKErrNotReady;
    }
    
    if (NULL == m_pFrameData) {
        return TTKErrArgument;
    }
    
    if (![EAGLContext setCurrentContext:m_pContext]) {
        return TTKErrArgument;
    }
    
    RenderToScreen();
    
    GetLastFrameInner(bIsTryGetFrame, pData);
    
    if (!bIsTryGetFrame) {
        RenderCommit();
    }
    
    return TTKErrNone;
}

int TTGLRenderES2::GetLastFrameInner(bool bIsTryGetFrame, void *pData)
{
    if (!bIsTryGetFrame) {
        return TTKErrNone;
    }
    
    NSInteger myDataLength = m_nBackingWidth * m_nBackingHeight * 4;
    
    if (NULL != m_pLastGetFrame) {
        free(m_pLastGetFrame);
        m_pLastGetFrame = NULL;
    }
    
    if (NULL == pData) {
        return TTKErrNone;
    }
    
    // allocate array and read pixels into it.
    m_pLastGetFrame = (GLubyte *) malloc(myDataLength);
    glReadPixels(0, 0, m_nBackingWidth, m_nBackingHeight, GL_RGBA, GL_UNSIGNED_BYTE, m_pLastGetFrame);
    
    // "
    /*
    pData->type = VO_IV_IMAGE_RGBA32;
    pData->height = m_nBackingHeight;
    pData->width = m_nBackingWidth;
    pData->size = myDataLength;
    pData->data = m_pLastGetFrame;
    */
    return TTKErrNone;
}

void TTGLRenderES2::UpdateVertices()
{
    if (0 == m_nBackingWidth || 0 == m_nBackingHeight || 0 == m_nTextureWidth || 0 == m_nTextureHeight) {
        return;
    }
    /*
    GLfloat nLengthX = (GLfloat)(2 * frame_draw_w) / m_BackingWidth;
    GLfloat nLengthY = (GLfloat)(2 * frame_draw_h) / m_BackingHeight;
    GLfloat nXLeft = -1 + (GLfloat)(2 * frame_draw_x) / m_BackingWidth; // from left
    GLfloat nYTop = 1 - ((GLfloat)(2 * frame_draw_y) / m_BackingHeight); // from top
    */
    //2 | 3
    //-   -
    //0 | 1
    if (m_nBackingWidth > m_nBackingHeight) {
        m_fSquareVertices[0] = -1;            // left X
        m_fSquareVertices[1] = -1;  // bottom Y
        
        m_fSquareVertices[2] = 1; // right X
        m_fSquareVertices[3] = -1;  // bottom Y
        
        m_fSquareVertices[4] = -1;            // left X
        m_fSquareVertices[5] = 1;             // top Y
        
        m_fSquareVertices[6] = 1; // right X
        m_fSquareVertices[7] = 1;             // top Y

    }
    else{
        m_fSquareVertices[0] = -1;            // left X
        m_fSquareVertices[1] = 0.1;//nYTop - nLengthY;  // bottom Y
        
        m_fSquareVertices[2] = 1; // right X
        m_fSquareVertices[3] = 0.1;//nYTop - nLengthY;  // bottom Y
        
        m_fSquareVertices[4] = -1;            // left X
        m_fSquareVertices[5] = 1;             // top Y
        
        m_fSquareVertices[6] = 1; // right X
        m_fSquareVertices[7] = 1;             // top Y
    }
    
    Redraw();
    
    /*
     //2 | 3
     //-   -
     //0 | 1
     m_fSquareVertices[0] = nXLeft;            // left X
     m_fSquareVertices[1] = nYTop - nLengthY;  // bottom Y
     
     m_fSquareVertices[2] = nXLeft + nLengthX; // right X
     m_fSquareVertices[3] = nYTop - nLengthY;  // bottom Y
     
     m_fSquareVertices[4] = nXLeft;            // left X
     m_fSquareVertices[5] = nYTop;             // top Y
     
     m_fSquareVertices[6] = nXLeft + nLengthX; // right X
     m_fSquareVertices[7] = nYTop;             // top Y
     */
}