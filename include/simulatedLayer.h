#pragma once

#include <map>
#include <vector>
#include <openxr/openxr.h>

#define XR_USE_GRAPHICS_API_OPENGL
#include <openxr/openxr_platform.h>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>


enum StreamingMode {
	PureWebsocket = 0,
	MappedFile = 1,
	SharedResource = 2,
	Ndi = 3
};


typedef struct {
    void* frameData;
    int64_t sizeInBytes;
} CompressedFrame;

typedef struct ExplicitLayer {
    void* frameData;
    int x;
    int y;
    int w;
    int h;
} ExplicitLayer;

class GLInterceptor {
	private:
		std::map<XrSwapchain, std::vector<XrSwapchainImageOpenGLKHR>> m_swapchainImages;
		std::map<XrSwapchain, int> m_currentSwapChainImage;
        std::vector<unsigned char> GLInterceptor::DownloadSwapChainSlowJustForTesting(XrSwapchain swapchain, int width, int height);
        void GLInterceptor::DownloadSwapChainSlow(XrSwapchain swapchain, int width, int height, ExplicitLayer& target);
        std::condition_variable frameReady;
        int64_t lastSubmittedFrame;
        CompressedFrame frames[2];
        std::mutex frameSubmission;
        ExplicitLayer layers[2];
    public:

		GLInterceptor(StreamingMode mode);
		~GLInterceptor();
        int startWebServer();
        int64_t WaitNewerFrame(int64_t lastFrame);
        void ReadFrame(std::function<void(CompressedFrame* frames, int count)> act);
		XRAPI_ATTR XrResult XRAPI_CALL xrCreateSwapchain(XrSession session, const XrSwapchainCreateInfo* createInfo, XrSwapchain* swapchain);
		XRAPI_ATTR XrResult XRAPI_CALL xrAcquireSwapchainImage(XrSwapchain swapchain, const XrSwapchainImageAcquireInfo* acquireInfo, uint32_t* index);
		XRAPI_ATTR XrResult XRAPI_CALL xrReleaseSwapchainImage(XrSwapchain swapchain, const XrSwapchainImageReleaseInfo* releaseInfo);
		XRAPI_ATTR XrResult XRAPI_CALL xrEndFrame(XrSession session, const XrFrameEndInfo* frameEndInfo);

};
