/************************************************************************
 *                                                                      *
 *        VisualOn, Inc. Confidential and Proprietary, 2003-            *
 *                                                                      *
 ************************************************************************/
/*******************************************************************************
 File:        TTGLRenderBase
 
 Contains:    VisualOn OpenGL ES Base Render cpp file
 
 Written by:  Jeff
 
 Change History (most recent first):
 2012-09-17   Jeff            Create file
 *******************************************************************************/

#import "TTGLRenderBase.h"
#import "TTMacrodef.h"
#import "TTLog.h"

TTGLRenderBase::TTGLRenderBase(EAGLContext* pContext)
:m_eOrientation(UIDeviceOrientationUnknown)
,m_eMode(UIViewContentModeScaleToFill)
,m_pContext(pContext)
,m_pUIViewSet(NULL)
,m_pVideoView(NULL)
,m_nTextureWidth(0)
,m_nTextureHeight(0)
,m_nBackingWidth(0)
,m_nBackingHeight(0)
{
    memset(&m_cTransform, 0, sizeof(m_cTransform));
    memset(&m_cFrame, 0, sizeof(m_cFrame));
    mCritical.Create();
}

TTGLRenderBase::~TTGLRenderBase()
{
    unInitVideoView();
    mCritical.Destroy();
}

int TTGLRenderBase::init()
{
    return TTKErrNone;
}

/*int TTGLRenderBase::SetLock(voCMutex *pLock)
{
    m_pLock = pLock;
    return TT_GL_RET_OK;
}*/

EAGLContext* TTGLRenderBase::GetGLContext()
{
    return m_pContext;
}

bool TTGLRenderBase::IsEqual(float a, float b)
{
    const static float V_RANGE = 0.000001;
    if (((a - b) > -V_RANGE)
        && ((a - b) < V_RANGE) ) {
        return true;
    }
    return false;
}

void TTGLRenderBase::Swap(float *a, float *b)
{
    if ((NULL == a) || (NULL == b)) {
        return;
    }
    
    float temp = *a;
    *a = *b;
    *b = temp;
}

bool TTGLRenderBase::IsViewChanged()
{
    if (NULL == m_pUIViewSet) {
        return true;
    }
    
    CGAffineTransform cTransform = [m_pUIViewSet transform];
    CGRect cFrame = [m_pUIViewSet frame];
    
    if (IsEqual(m_cTransform.a, cTransform.a)
        && IsEqual(m_cTransform.b, cTransform.b)
        && IsEqual(m_cTransform.c, cTransform.c)
        && IsEqual(m_cTransform.d, cTransform.d)
        && IsEqual(m_cTransform.tx, cTransform.tx)
        && IsEqual(m_cTransform.ty, cTransform.ty)
        && IsEqual(m_cFrame.origin.x, cFrame.origin.x)
        && IsEqual(m_cFrame.origin.y, cFrame.origin.y)
        && IsEqual(m_cFrame.size.width, cFrame.size.width)
        && IsEqual(m_cFrame.size.height, cFrame.size.height)
        && m_eOrientation == [[UIDevice currentDevice] orientation]
        && m_eMode == m_pUIViewSet.contentMode) {
        return false;
    }
    
    if (m_eOrientation != [[UIDevice currentDevice] orientation]) {
        m_eOrientation = [[UIDevice currentDevice] orientation];
    }
    
    return true;
}

int TTGLRenderBase::SetView(UIView* view)
{
    bool bReInitVideoView = false;
    {
        TTCAutoLock Lock(&mCritical);
        
        if (view != m_pUIViewSet) {
            bReInitVideoView = true;
        }
        // return if view not be changed
        else if (!IsViewChanged()) {
            return TTKErrNone;
        }
        
        if (NULL != view) {
            m_pUIViewSet = view;
        }
        
        if (NULL == m_pUIViewSet) {
            return TTKErrArgument;
        }
        
        m_eOrientation = [[UIDevice currentDevice] orientation];
       
        //屏幕旋转通过view的矩阵变化实现,如CGAffineTransformScale(view.transform, 0.5, 0.5)
        //当设备监测到旋转时，会通知当前程序，当前程序再通知程序中的window，window会通知它的rootViewController的，rootViewController对其view的transform进行设置，最终完成旋转。
        m_cTransform = [m_pUIViewSet transform];
        
        m_cFrame = [m_pUIViewSet frame];
        m_eMode = m_pUIViewSet.contentMode;
        
        //LOGI("view:%d, a:%f,b:%f,c:%f,d:%f,tx:%f,ty:%f\n", (int )view, m_cTransform.a, m_cTransform.b, m_cTransform.c, m_cTransform.d, m_cTransform.tx, m_cTransform.ty);
    }
    
   // if ([NSRunLoop mainRunLoop] == [NSRunLoop currentRunLoop]) {
    {
        TTCAutoLock Lock(&mCritical);
        initVideoView(bReInitVideoView);
        RefreshView();
    }
   // }
    //else {
        
        //PostRunOnMainRequest(false, bReInitVideoView, NULL, NULL);
   // }
    
    return TTKErrNone;
}

int TTGLRenderBase::initVideoView(bool bReInit)
{
    TTCAutoLock Lock(&mCritical);
    
    if (bReInit) {
        unInitVideoView();
    }
    
    if (nil == m_pVideoView) {
        
        CGRect cRect = m_pUIViewSet.bounds;

        m_pVideoView = [[TTVideoView alloc] initWithFrame:cRect];

        [m_pVideoView setAutoresizingMask:(UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight)];
        
        [m_pUIViewSet insertSubview:m_pVideoView atIndex:0];
    }
    
    LOGI("mode:%d, width:%f, height:%f", m_pUIViewSet.contentMode, m_pVideoView.bounds.size.width, m_pVideoView.bounds.size.height);
    
    if (UIViewContentModeScaleToFill == m_pUIViewSet.contentMode) {
        if (m_pVideoView.bounds.size.width > m_pVideoView.bounds.size.height) {
            m_pVideoView.contentMode = UIViewContentModeScaleAspectFit;
        }
        else {
            m_pVideoView.contentMode = UIViewContentModeScaleAspectFill;
        }
    }
    else {
        m_pVideoView.contentMode = m_pUIViewSet.contentMode;
    }
    
    return TTKErrNone;
}

int TTGLRenderBase::unInitVideoView()
{
    if (nil != m_pVideoView) {
        [m_pVideoView removeFromSuperview];
        [m_pVideoView release];
        m_pVideoView = NULL;
    }
    
    return TTKErrNone;
}

int TTGLRenderBase::RenderYUV(TTVideoBuffer *pVideoBuffer)
{
    return 0;
}

unsigned char* TTGLRenderBase::GetFrameData()
{
    return NULL;
}

int TTGLRenderBase::RenderFrameData()
{
    return 0;
}

int TTGLRenderBase::GetSupportType()
{
    return TT_GL_SUPPORT_NONE;
}

bool TTGLRenderBase::IsGLRenderReady() {
    return false;
}

void TTGLRenderBase::UpdateVertices()
{
    
}

int TTGLRenderBase::GetLastFrame(void *pData)//VO_IMAGE_DATA
{
    return 0;
}
