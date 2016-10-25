#ifndef __VO_GLRENDER_ES2__
#define __VO_GLRENDER_ES2__

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/glext.h>
#import "TTGLRenderBase.h"

class TTGLRenderES2 : public TTGLRenderBase {
    
    friend class TTGLRenderFactory;
    
public:
    // should add lock
    
    virtual int SetTexture(int nWidth, int nHeight);
    //virtual int SetOutputRect(int nLeft, int nTop, int nRight, int nBottom);

    virtual int ClearGL();
    
    virtual int Redraw();
    virtual int RenderYUV(TTVideoBuffer *pVideoBuffer);
    
    virtual int GetSupportType();
    
    virtual int GetLastFrame(void *pData);//VO_IMAGE_DATA
    
    //int     GetLastFrameInner(bool bIsTryGetFrame, void *pData);
    //int     RedrawInner(bool bIsTryGetFrame, void *pData);
    //int     UploadTexture(TTVideoBuffer *pVideoBuffer);
    
//protected:
    // should add lock
    virtual int RefreshView();
    
//protected:
    TTGLRenderES2(EAGLContext* pContext);
    virtual ~TTGLRenderES2();
    
    virtual int init();
    
    virtual void UpdateVertices();
    virtual int SetupGlViewport();
    virtual int SetupRenderBuffer();
    virtual int SetupFrameBuffer();
    virtual int SetupTexture();
    
    virtual int TextureSizeChange();
    
    virtual int DeleteRenderBuffer();
    virtual int DeleteFrameBuffer();
    virtual int DeleteTexture();
    
    virtual int GLTexImage2D(GLuint nTexture, Byte *pDate, int nWidth, int nHeight);

    virtual int CompileAllShaders();
    virtual GLuint CompileShader(const GLchar *pBuffer, GLenum shaderType);
    
    virtual int UploadTexture(TTVideoBuffer *pVideoBuffer);
    virtual int RenderToScreen();
    virtual int RenderCommit();
    
    virtual int RedrawInner(bool bIsTryGetFrame, void *pData);
    virtual int GetLastFrameInner(bool bIsTryGetFrame, void *pData);
    
    virtual bool IsGLRenderReady();
    
protected:
    int m_nPositionSlot;
    int m_nTexCoordSlot;
    
    GLuint m_nProgramHandle;
    
    GLfloat m_fTextureVertices[8];
    GLfloat m_fSquareVertices[8];
    
private:
    bool m_bInitSuccess;
    
    float m_fOutLeftPer;
    float m_fOutTopPer;
    float m_fOutRightPer;
    float m_fOutBottomPer;
    
    GLuint m_nColorRenderBuffer;
    GLuint m_nFrameBuffer;
    
    GLuint m_nTexturePlanarY;
    GLuint m_nTexturePlanarU;
    GLuint m_nTexturePlanarV;
    
    GLint m_nTextureUniformY;
    GLint m_nTextureUniformU;
    GLint m_nTextureUniformV;
    
    unsigned char* m_pFrameData;
    
    GLubyte* m_pLastGetFrame;
    
    GLfloat frame_draw_x;
    GLfloat frame_draw_y;
    GLfloat frame_draw_h;
    GLfloat frame_draw_w;
};

#endif

