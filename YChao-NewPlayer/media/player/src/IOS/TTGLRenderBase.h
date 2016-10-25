#ifndef __TT_GLRENDER_BASE__
#define __TT_GLRENDER_BASE__

#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import "TTVideoView.h"

#include "TTCritical.h"
#include "TTMediadef.h"

static const int TT_GL_SUPPORT_NONE   = 0;
static const int TT_GL_SUPPORT_RGB    = 0x1;
static const int TT_GL_SUPPORT_Y_U_V  = 0x1 << 1;
static const int TT_GL_SUPPORT_Y_UV   = 0x1 << 2;
    

// Effective C++ 42: Use private inheritance judiciously
class TTGLRenderBase
{
     friend class TTGLRenderFactory;
public:
    
    EAGLContext *GetGLContext();
    //int SetLock(voCMutex *pLock);
    
    virtual int SetView(UIView *view);
    
    virtual int SetTexture(int nWidth, int nHeight) = 0;
    //virtual int SetOutputRect(int nLeft, int nTop, int nRight, int nBottom) = 0;
    
    virtual int ClearGL() = 0;
    
    virtual int Redraw() = 0;
    virtual int RenderYUV(TTVideoBuffer *pVideoBuffer) ;

    virtual int RenderFrameData();

    virtual unsigned char* GetFrameData();
    
    virtual int GetSupportType();
    
    virtual int GetLastFrame(void *pData) ;//VO_IMAGE_DATA
    
//protected:
    
    TTGLRenderBase(EAGLContext* pContext);
    virtual ~TTGLRenderBase();
    
    virtual int init();
    
    virtual int unInitVideoView();
    
    virtual int initVideoView(bool bReInit);
    
    virtual int RefreshView() = 0;
    
    virtual bool IsViewChanged();
    
    virtual bool IsGLRenderReady();
    
    bool IsEqual(float a, float b);
    
    void Swap(float *a, float *b);
    
    virtual void UpdateVertices();
    
//protected:
    UIDeviceOrientation m_eOrientation;
    UIViewContentMode m_eMode;
    
    EAGLContext *m_pContext;
    UIView *m_pUIViewSet;
    TTVideoView *m_pVideoView;
    RTTCritical mCritical;
    
    /* Dimensions of the backing buffer */
	int m_nBackingWidth;
    int m_nBackingHeight;
    
    /* Dimensions of the texture in pixels */
	int m_nTextureWidth;
	int m_nTextureHeight;
    
    CGAffineTransform m_cTransform;
    CGRect            m_cFrame;
};

#endif

