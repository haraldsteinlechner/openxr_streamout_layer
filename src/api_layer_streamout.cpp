#include "loader_interfaces.h"

#include <cstring>
#include <iostream>
#include <chrono>
using namespace std::chrono;

#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffers_iterator.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

#include "simulatedLayer.h"

#include <GL/GL.h>
#include <common/gfxwrapper_opengl.h>
#include "common.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


//#define XR_USE_PLATFORM_XLIB
#define XR_USE_GRAPHICS_API_OPENGL

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

static const char* gl_api_string = nullptr;


const char* _layerName = "XR_aardworx_streamout";
#define log std::cout << _layerName << ":: "



static PFN_xrCreateSession _nextxrCreateSession;
static PFN_xrEndFrame _nextxrEndFrame;
static PFN_xrCreateSwapchain _nextxrCreateSwapchain;
static PFN_xrAcquireSwapchainImage _nextxrAcquireSwapchainImage;
static PFN_xrWaitSwapchainImage _nextxrWaitSwapchainImage;
static PFN_xrReleaseSwapchainImage _nextxrReleaseSwapchainImage;
static PFN_xrEnumerateSwapchainImages _nextxrEnumerateSwapchainImages;

// load next function pointers in _xrCreateApiLayerInstance
PFN_xrGetInstanceProcAddr _nextXrGetInstanceProcAddr = NULL;

// cache create infos
static XrInstanceCreateInfo instanceInfo;
static XrInstance xrInstance;

static GLInterceptor* interceptor;
static bool intercept = true;

static XRAPI_ATTR XrResult XRAPI_CALL
_xrCreateSession(XrInstance instance, const XrSessionCreateInfo* createInfo, XrSession* session)
{
	log << __FUNCTION__ << std::endl;
	const XrBaseInStructure* s = (const XrBaseInStructure*)createInfo;
	while (s) {
		if (s->type == XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR) {
			//gl_api = new GLXApi((struct XrGraphicsBindingOpenGLXlibKHR *)s);
			interceptor = new GLInterceptor(PureWebsocket);
			interceptor->startWebServer();
		}
		s = s->next;
	}

	if (interceptor == nullptr) {
		log << "could not find graphics binding. only OpenGL Win32 supported currently" << std::endl;
		throw std::runtime_error("graphics binding not supported.");
	}

	XrResult res = _nextxrCreateSession(instance, createInfo, session);
	return res;
}


static XRAPI_ATTR XrResult XRAPI_CALL
_xrEndFrame(XrSession session, const XrFrameEndInfo* frameEndInfo)
{
#ifdef VERBOSE
	log << __FUNCTION__ << std::endl;
#endif

	XrResult res = intercept ? interceptor->xrEndFrame(session, frameEndInfo) : _nextxrEndFrame(session, frameEndInfo);

	return res;
}

static XRAPI_ATTR XrResult XRAPI_CALL
_xrCreateSwapchain(XrSession session, const XrSwapchainCreateInfo* createInfo, XrSwapchain* swapchain)
{
#ifdef VERBOSE
	log << __FUNCTION__ << std::endl;
#endif

	XrResult res = intercept ? interceptor->xrCreateSwapchain(session, createInfo, swapchain) : _nextxrCreateSwapchain(session, createInfo, swapchain);

	return res;
}

static XRAPI_ATTR XrResult XRAPI_CALL
_xrAcquireSwapchainImage(XrSwapchain swapchain, const XrSwapchainImageAcquireInfo* acquireInfo, uint32_t* index)
{
#ifdef VERBOSE
	log << __FUNCTION__ << std::endl;
#endif

	XrResult res = intercept ? interceptor->xrAcquireSwapchainImage(swapchain, acquireInfo, index) : _nextxrAcquireSwapchainImage(swapchain, acquireInfo, index);

	return res;
}

static XRAPI_ATTR XrResult XRAPI_CALL
_xrWaitSwapchainImage(XrSwapchain swapchain, const XrSwapchainImageWaitInfo* waitInfo)
{
#ifdef VERBOSE
	log << __FUNCTION__ << std::endl;
#endif

	XrResult res =  _nextxrWaitSwapchainImage(swapchain, waitInfo);

	return res;
}

static XRAPI_ATTR XrResult XRAPI_CALL
_xrReleaseSwapchainImage(XrSwapchain swapchain, const XrSwapchainImageReleaseInfo* releaseInfo)
{
#ifdef VERBOSE
	log << __FUNCTION__ << std::endl;
#endif

	XrResult res = interceptor ? interceptor->xrReleaseSwapchainImage(swapchain, releaseInfo) : _nextxrReleaseSwapchainImage(swapchain, releaseInfo);

	return res;
}

static XRAPI_ATTR XrResult XRAPI_CALL
_xrGetInstanceProcAddr(XrInstance instance, const char* name, PFN_xrVoidFunction* function)
{
	// log << ": " << name << std::endl;

	std::string func_name = name;

	if (func_name == "xrCreateSession") {
		*function = (PFN_xrVoidFunction)_xrCreateSession;
		return XR_SUCCESS;
	}

	if (func_name == "xrEndFrame") {
		*function = (PFN_xrVoidFunction)_xrEndFrame;
		return XR_SUCCESS;
	}

	if (func_name == "xrCreateSwapchain") {
		*function = (PFN_xrVoidFunction)_xrCreateSwapchain;
		return XR_SUCCESS;
	}

	if (func_name == "xrAcquireSwapchainImage") {
		*function = (PFN_xrVoidFunction)_xrAcquireSwapchainImage;
		return XR_SUCCESS;
	}

	if (func_name == "xrWaitSwapchainImage") {
		*function = (PFN_xrVoidFunction)_xrWaitSwapchainImage;
		return XR_SUCCESS;
	}

	if (func_name == "xrxrReleaseSwapchainImage") {
		*function = (PFN_xrVoidFunction)_xrReleaseSwapchainImage;
		return XR_SUCCESS;
	}


	return _nextXrGetInstanceProcAddr(instance, name, function);
}

// xrCreateInstance is a special case that we can't hook. We get this amended call instead.
static XrResult XRAPI_PTR
_xrCreateApiLayerInstance(const XrInstanceCreateInfo* info,
	const XrApiLayerCreateInfo* apiLayerInfo,
	XrInstance* instance)
{
	_nextXrGetInstanceProcAddr = apiLayerInfo->nextInfo->nextGetInstanceProcAddr;
	XrResult result;


	// first let the instance be created
	result = apiLayerInfo->nextInfo->nextCreateApiLayerInstance(info, apiLayerInfo, instance);
	if (XR_FAILED(result)) {
		log << "Failed to load xrCreateActionSet" << std::endl;
		return result;
	}

	// remember which opengl extension is enabled
	for (uint32_t i = 0; i < info->enabledExtensionCount; i++) {
		if (strcmp(info->enabledExtensionNames[i], XR_KHR_OPENGL_ENABLE_EXTENSION_NAME) == 0) {
			log << "graphics binding: " << XR_KHR_OPENGL_ENABLE_EXTENSION_NAME;
			gl_api_string = XR_KHR_OPENGL_ENABLE_EXTENSION_NAME;
		}
	}


	// then use the created instance to load next function pointers
	result = _nextXrGetInstanceProcAddr(*instance, "xrCreateSession", (PFN_xrVoidFunction*)&_nextxrCreateSession);
	if (XR_FAILED(result)) {
		log << "Failed to load xrCreateSession" << std::endl;
		return result;
	}


	// the 5 functions that need to be fixed up https://developer.blender.org/T92723#1257606
	result = _nextXrGetInstanceProcAddr(*instance, "xrEndFrame", (PFN_xrVoidFunction*)&_nextxrEndFrame);
	if (XR_FAILED(result)) {
		log << "Failed to load xrEndFrame" << std::endl;
		return result;
	}

	result =
		_nextXrGetInstanceProcAddr(*instance, "xrCreateSwapchain", (PFN_xrVoidFunction*)&_nextxrCreateSwapchain);
	if (XR_FAILED(result)) {
		log << "Failed to load xrCreateSwapchain" << std::endl;
		return result;
	}

	result = _nextXrGetInstanceProcAddr(*instance, "xrAcquireSwapchainImage",
		(PFN_xrVoidFunction*)&_nextxrAcquireSwapchainImage);
	if (XR_FAILED(result)) {
		log << "Failed to load xrAcquireSwapchainImage" << std::endl;
		return result;
	}

	result = _nextXrGetInstanceProcAddr(*instance, "xrWaitSwapchainImage",
		(PFN_xrVoidFunction*)&_nextxrWaitSwapchainImage);
	if (XR_FAILED(result)) {
		log << "Failed to load xrWaitSwapchainImage" << std::endl;
		return result;
	}

	result = _nextXrGetInstanceProcAddr(*instance, "xrReleaseSwapchainImage",
		(PFN_xrVoidFunction*)&_nextxrReleaseSwapchainImage);
	if (XR_FAILED(result)) {
		log << "Failed to load xrReleaseSwapchainImage" << std::endl;
		return result;
	}

	result = _nextXrGetInstanceProcAddr(*instance, "xrEnumerateSwapchainImages",
		(PFN_xrVoidFunction*)&_nextxrEnumerateSwapchainImages);
	if (XR_FAILED(result)) {
		log << "Failed to load xrEnumerateSwapchainImages" << std::endl;
		return result;
	}


	instanceInfo = *info;
	xrInstance = *instance;
	log << ": Created api layer instance for app " << info->applicationInfo.applicationName << std::endl;

	return result;
}

extern "C" {

#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

	EXTERN_DLL_EXPORT  XrResult
		xrNegotiateLoaderApiLayerInterface(const XrNegotiateLoaderInfo* loaderInfo,
			const char* layerName,
			XrNegotiateApiLayerRequest* apiLayerRequest)
	{
		_layerName = strdup(layerName);

		log << ": Using API layer: " << layerName << std::endl;

		log << ": loader API version min: " << XR_VERSION_MAJOR(loaderInfo->minApiVersion) << "."
			<< XR_VERSION_MINOR(loaderInfo->minApiVersion) << "." << XR_VERSION_PATCH(loaderInfo->minApiVersion) << "."
			<< " max: " << XR_VERSION_MAJOR(loaderInfo->maxApiVersion) << "."
			<< XR_VERSION_MINOR(loaderInfo->maxApiVersion) << "." << XR_VERSION_PATCH(loaderInfo->maxApiVersion) << "."
			<< std::endl;

		log << ": loader interface version min: " << XR_VERSION_MAJOR(loaderInfo->minInterfaceVersion) << "."
			<< XR_VERSION_MINOR(loaderInfo->minInterfaceVersion) << "."
			<< XR_VERSION_PATCH(loaderInfo->minInterfaceVersion) << "."
			<< " max: " << XR_VERSION_MAJOR(loaderInfo->maxInterfaceVersion) << "."
			<< XR_VERSION_MINOR(loaderInfo->maxInterfaceVersion) << "."
			<< XR_VERSION_PATCH(loaderInfo->maxInterfaceVersion) << "." << std::endl;



		// TODO: proper version check
		apiLayerRequest->layerInterfaceVersion = loaderInfo->maxInterfaceVersion;
		apiLayerRequest->layerApiVersion = loaderInfo->maxApiVersion;
		apiLayerRequest->getInstanceProcAddr = _xrGetInstanceProcAddr;
		apiLayerRequest->createApiLayerInstance = _xrCreateApiLayerInstance;

		return XR_SUCCESS;
	}
}


static bool useInterception = true;

XRAPI_ATTR XrResult XRAPI_CALL GLInterceptor::xrCreateSwapchain(XrSession session, const XrSwapchainCreateInfo* createInfo, XrSwapchain* swapchain) {
	auto result = _nextxrCreateSwapchain(session, createInfo, swapchain);
	if (useInterception)
	{
		uint32_t imageCount;
		CHECK_XRCMD(_nextxrEnumerateSwapchainImages(*swapchain, 0, &imageCount, nullptr));
		std::vector<XrSwapchainImageOpenGLKHR> swapchainImageBuffer(imageCount, { XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR });
		std::vector<XrSwapchainImageBaseHeader*> swapchainImageBase;
		for (XrSwapchainImageOpenGLKHR& image : swapchainImageBuffer) {
			swapchainImageBase.push_back(reinterpret_cast<XrSwapchainImageBaseHeader*>(&image));
		}
		CHECK_XRCMD(_nextxrEnumerateSwapchainImages(*swapchain, imageCount, &imageCount, swapchainImageBase[0]));

		m_swapchainImages.insert(std::make_pair(*swapchain, std::move(swapchainImageBuffer)));
	}
	return result;
}


XRAPI_ATTR XrResult XRAPI_CALL GLInterceptor::xrAcquireSwapchainImage(XrSwapchain swapchain, const XrSwapchainImageAcquireInfo* acquireInfo, uint32_t* index) {
	XrResult res = _nextxrAcquireSwapchainImage(swapchain, acquireInfo, index);
	m_currentSwapChainImage.insert_or_assign(swapchain, *index);
	return res;
}


std::vector<unsigned char> GLInterceptor::DownloadSwapChainSlowJustForTesting(XrSwapchain swapchain, int width, int height) {
	auto images = m_swapchainImages[swapchain];
	//auto currentImage = m_currentSwapChainImage[swapchain];
	XrSwapchainImageOpenGLKHR& image = images[m_currentSwapChainImage[swapchain]];
	glBindTexture(GL_TEXTURE_2D, image.image);
	int w, h = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
	assert(w == width && h == height);
	std::vector<unsigned char> frameData(width * height * 4);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, &frameData[0]);
	return frameData;
}
void GLInterceptor::DownloadSwapChainSlow(XrSwapchain swapchain, int width, int height,
	ExplicitLayer& target) {
	auto images = m_swapchainImages[swapchain];
	//auto currentImage = m_currentSwapChainImage[swapchain];
	XrSwapchainImageOpenGLKHR& image = images[m_currentSwapChainImage[swapchain]];
	glBindTexture(GL_TEXTURE_2D, image.image);
	int w, h = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
	assert(w == width && h == height);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, target.frameData);
	target.x = 0;
	target.y = 0;
	target.w = width;
	target.h = height;
}

XRAPI_ATTR XrResult XRAPI_CALL GLInterceptor::xrReleaseSwapchainImage(XrSwapchain swapchain, const XrSwapchainImageReleaseInfo* releaseInfo) {
	XrResult res = _nextxrReleaseSwapchainImage(swapchain, releaseInfo);
	return res;
}

typedef struct {
	GLInterceptor* t;
	uint32_t index;
	int offset;
} Clo;

XRAPI_ATTR XrResult XRAPI_CALL GLInterceptor::xrEndFrame(XrSession session, const XrFrameEndInfo* frameEndInfo) {
#ifdef VERBOSE
	log << __FUNCTION__ << std::endl;
#endif

	if (frameEndInfo->layerCount > 0) {
		const XrCompositionLayerProjection* layer = reinterpret_cast<const XrCompositionLayerProjection*>(frameEndInfo->layers[0]);
		for (uint32_t i = 0; i < layer->viewCount; i++) {
			auto view = layer->views[i];
			assert(view.subImage.imageRect.offset.x == 0 && view.subImage.imageRect.offset.y == 0);

			if (false) {
				auto data = DownloadSwapChainSlowJustForTesting(view.subImage.swapchain, view.subImage.imageRect.extent.width, view.subImage.imageRect.extent.height);
				char buff[256];  // testing only
				sprintf(buff, "C:\\temp\\img_%d_%d.jpg", (int)view.subImage.swapchain, i);
				stbi_write_jpg((const char*)buff, view.subImage.imageRect.extent.width, view.subImage.imageRect.extent.height, 4,
					&data[0], 100);
			}

			// can be optimized for faster download
			DownloadSwapChainSlow(view.subImage.swapchain, view.subImage.imageRect.extent.width,
				view.subImage.imageRect.extent.height, layers[i]);
		}
	}

	{
		std::unique_lock lk(frameSubmission);

		if (frameEndInfo->layerCount > 0) {
			auto start = high_resolution_clock::now();
			const XrCompositionLayerProjection* layer =
				reinterpret_cast<const XrCompositionLayerProjection*>(frameEndInfo->layers[0]);

			for (uint32_t i = 0; i < layer->viewCount; i++) {
				Clo o = { this, i, 0 };
				auto copyToCompressedFbo = [](void* context, void* data, int size) {
					auto clo = (Clo*)context;
					auto start = (unsigned char*)clo->t->frames[clo->index].frameData + clo->offset;
					memcpy(start, data, size);
					clo->offset += size;
				};
				stbi_write_jpg_to_func(copyToCompressedFbo, static_cast<void*>(&o), layers[i].w, layers[i].h, 4,
					layers[i].frameData, 90);
				frames[i].sizeInBytes = o.offset;
			}
			auto duration = duration_cast<milliseconds>(high_resolution_clock::now() - start);

			this->lastSubmittedFrame++;
			frameReady.notify_all();
#if VERBOSE
			std::cout << "[LAYER] frame id: " << this->lastSubmittedFrame << ", took: " << duration.count() << " ms." << std::endl;
#endif

		}
	}

	XrResult res = _nextxrEndFrame(session, frameEndInfo);

	return res;
}


int64_t GLInterceptor::WaitNewerFrame(int64_t lastFrame) {
	std::unique_lock lk(frameSubmission);
	frameReady.wait(lk, [this, &lastFrame] {
		return lastFrame <= this->lastSubmittedFrame;
	});
	return this->lastSubmittedFrame;
}

void GLInterceptor::ReadFrame(std::function<void(CompressedFrame* frames, int count)> act) {
	frameSubmission.lock();
	act(this->frames, 2);
	frameSubmission.unlock();
}

int GLInterceptor::startWebServer() {

	auto do_listen = [this]() {
		try {
			auto const address = net::ip::make_address("127.0.0.1");
			auto const port = 4325;

			net::io_context ioc{ 1 };

			tcp::acceptor acceptor{ ioc, {address, port} };
			for (;;) {
				tcp::socket socket{ ioc };

				acceptor.accept(socket);

				auto do_session = [this](tcp::socket socket) {
					try {
						websocket::stream<tcp::socket> ws{ std::move(socket) };

						// Set a decorator to change the Server of the handshake
						ws.set_option(websocket::stream_base::decorator([](websocket::response_type& res) {
							res.set(http::field::server, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-sync");
						}));

						ws.accept();

						beast::flat_buffer buf;
						ws.read(buf);
						auto data = buf.data();
						std::string s(buffers_begin(data), buffers_begin(data) + buf.size());
						std::cout << "ws connected with: " << s << std::endl;

						int64_t frameId = -1;
						ws.text(false);

						for (;;) {

							frameId = this->WaitNewerFrame(frameId);
							this->ReadFrame([&ws](CompressedFrame* frames, int count) {
								ws.write(boost::asio::buffer((const void*)frames[0].frameData, frames[0].sizeInBytes));
								ws.write(boost::asio::buffer((const void*)frames[1].frameData, frames[1].sizeInBytes));
							});

							beast::flat_buffer result;
							ws.read(result);
							auto resultData = result.data();
							std::string answer(buffers_begin(resultData), buffers_begin(resultData) + result.size());
							if (answer == "UPDATEDTEXTURES" || answer == "COULDNOTUPDATE") {
							}
							else {
								std::cout << "protocol error (2)" << std::endl;
							}

						}
					}
					catch (beast::system_error const& se) {
						if (se.code() != websocket::error::closed) std::cerr << "Error: " << se.code().message() << std::endl;
					}
					catch (std::exception const& e) {
						std::cerr << "Error: " << e.what() << std::endl;
					}
				};
				std::thread(do_session, std::move(socket)).detach();


			}
		}
		catch (const std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
		}
	};

	std::thread(do_listen).detach();

	return 0;
}

GLInterceptor::GLInterceptor(StreamingMode mode) {
	// TODO manual resizing instead of those buffers... (use them for proof of concept)
	frames[0].frameData = malloc(4096ULL * 4096ULL * 4ULL);
	frames[1].frameData = malloc(4096ULL * 4096ULL * 4ULL);
	layers[0].frameData = malloc(4096ULL * 4096ULL * 4ULL);
	layers[1].frameData = malloc(4096ULL * 4096ULL * 4ULL);
	frames[0].sizeInBytes = 0;
	frames[1].sizeInBytes = 0;
	lastSubmittedFrame = 0;

}

GLInterceptor::~GLInterceptor() {
	for (int i = 0; i < 2; i++) {
		free(frames[i].frameData);
		free(layers[i].frameData);
	}
}
