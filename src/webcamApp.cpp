#include "cinder/Cinder.h"
#include "cinder/app/AppNative.h"
#include "cinder/gl/Texture.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"
#include "msw/videoInput/videoInput.h"
#include "cinder/Json.h"
#include <vector>

using namespace ci;
using namespace ci::app;

class WebCamApp : public AppNative {
public:
	void setup();

	void update();
	void draw();
	void prepareSettings(Settings* settings) {
		::AllocConsole();
		::FILE* fp;
		::freopen_s(&fp, "CON", "w", stdout);

		//settings->enableConsoleWindow();
		settings->setWindowSize(width, height);

		const std::vector<std::string> args = ci::app::AppNative::getArgs();
		if (args.size() == 2) {
			int numDevs = videoInput::listDevices();
			videoInput::setVerbose(false);
			dev = 0;
			VI.setUseCallback(true);
			VI.setupDevice(dev, width, height);
			
			std::cout << args[1] << std::endl;
			if (boost::filesystem::exists(args[1])) {
				loadCamParams(args[1]);
			}
			VI.stopDevice(dev);
			::exit(1);
		}
	}

	void saveCamParamsCB()
	{
		ci::JsonTree doc;
		doc.addChild(ci::JsonTree("camera name", VI.getDeviceName(dev)));
		doc.addChild(ci::JsonTree("brightness", brightness));
		doc.addChild(ci::JsonTree("contrast", contrast));
		doc.addChild(ci::JsonTree("gain", gain));
		doc.addChild(ci::JsonTree("saturation", saturation));
		doc.addChild(ci::JsonTree("sharpness", sharpness));
		doc.addChild(ci::JsonTree("whiteBalance", whiteBalance));
		doc.addChild(ci::JsonTree("backlightCompensation", backlightCompensation));
		doc.addChild(ci::JsonTree("focus", focus));
		doc.addChild(ci::JsonTree("exposure", exposure));
		ci::app::console() << doc;
		doc.write("param.json");
	}
	
	void loadCamParams(const std::string fname)
	{
		ci::JsonTree doc(ci::loadFile(fname));
		std::cout << doc;

		brightness = doc.getChild("brightness").getValue<int32_t>();
		gain = doc.getChild("gain").getValue<int32_t>();
		exposure = doc.getChild("exposure").getValue<int32_t>();
		contrast = doc.getChild("contrast").getValue<int32_t>();
		saturation = doc.getChild("saturation").getValue<int32_t>();
		sharpness = doc.getChild("sharpness").getValue<int32_t>();
		focus = doc.getChild("focus").getValue<int32_t>();
		backlightCompensation = doc.getChild("backlightCompensation").getValue<int32_t>();
		whiteBalance = doc.getChild("whiteBalance").getValue<int32_t>();

		VI.setVideoSettingFilter(dev, VI.propBrightness, brightness, 0x0002);
		VI.setVideoSettingFilter(dev, VI.propGain, gain, 0x0002);
		VI.setVideoSettingCamera(dev, VI.propExposure, exposure, 0x0002);
		VI.setVideoSettingFilter(dev, VI.propContrast, contrast, 0x0002);
		VI.setVideoSettingFilter(dev, VI.propSaturation, saturation, 0x0002);
		VI.setVideoSettingFilter(dev, VI.propSharpness, sharpness, 0x0002);
		VI.setVideoSettingCamera(dev, VI.propFocus, focus, 0x0002);
		VI.setVideoSettingFilter(dev, VI.propBacklightCompensation, backlightCompensation, 0x0002);
		VI.setVideoSettingFilter(dev, VI.propWhiteBalance, whiteBalance, 0x0002);
	}

	void loadCamParamsCB()
	{
		if (boost::filesystem::exists("param.json")) {
			loadCamParams("param.json");
		}
	}

	WebCamApp() : width(1280), height(720) {}

	void shutdown() {
		VI.stopDevice(dev);
	}
	
	gl::TextureRef		mTexture;
	ci::Surface			mSurface;
	videoInput			VI;
	int					dev;
	ci::params::InterfaceGlRef	mParams;

	int width;
	int height;

	int32_t brightness;
	int32_t contrast;
	int32_t gain;
	int32_t saturation;
	int32_t sharpness;
	int32_t whiteBalance;
	int32_t backlightCompensation;
	int32_t focus;
	int32_t exposure;
};

void WebCamApp::setup()
{
	int numDevs = videoInput::listDevices();
	videoInput::setVerbose(false);
	dev = 0;
	VI.setUseCallback(true);
	VI.setupDevice(dev, width, height);

	mSurface = ci::Surface(width, height, false, SurfaceChannelOrder::RGB);
	mParams = ci::params::InterfaceGl::create(ci::app::getWindow(), "WEBCAM Setting", ci::Vec2i(200, 250));

	mParams->addText(VI.getDeviceName(dev));
		
	mParams->addSeparator();
	long lmin, lmax, lsd, lc, lf, ldv;

	// == Brightness ==
	VI.getVideoSettingFilter(dev, VI.propBrightness, lmin, lmax, lsd, lc, lf, ldv);
	brightness = lc;
	mParams->addParam("brightness", &brightness).updateFn([this](){
		VI.setVideoSettingFilter(dev, VI.propBrightness, brightness, 0x0002);
	}).min(lmin).max(lmax).step(lsd);

	// == Gain ==
	VI.getVideoSettingFilter(dev, VI.propGain, lmin, lmax, lsd, lc, lf, ldv);
	gain = lc;
	mParams->addParam("gain", &gain).updateFn([this](){
		VI.setVideoSettingFilter(dev, VI.propGain, gain, 0x0002);
	}).min(lmin).max(lmax).step(lsd);

	// == Exposure ==
	VI.getVideoSettingCamera(dev, VI.propExposure, lmin, lmax, lsd, lc, lf, ldv);
	exposure = lc;
	mParams->addParam("exposure", &exposure).updateFn([this](){
		VI.setVideoSettingCamera(dev, VI.propExposure, exposure, 0x0002);
	}).min(lmin).max(lmax).step(lsd);

	// == Contrast ==
	VI.getVideoSettingFilter(dev, VI.propContrast, lmin, lmax, lsd, lc, lf, ldv);
	contrast = lc;
	mParams->addParam("contrast", &contrast).updateFn([this](){
		VI.setVideoSettingFilter(dev, VI.propContrast, contrast, 0x0002);
	}).min(lmin).max(lmax).step(lsd);

	// == Saturation ==
	VI.getVideoSettingFilter(dev, VI.propSaturation, lmin, lmax, lsd, lc, lf, ldv);
	saturation = lc;
	mParams->addParam("saturation", &saturation).updateFn([this](){
		VI.setVideoSettingFilter(dev, VI.propSaturation, saturation, 0x0002);
	}).min(lmin).max(lmax).step(lsd);

	// == Sharpness ==
	VI.getVideoSettingFilter(dev, VI.propSharpness, lmin, lmax, lsd, lc, lf, ldv);
	sharpness = lc;
	mParams->addParam("sharpness", &sharpness).updateFn([this](){
		VI.setVideoSettingFilter(dev, VI.propSharpness, sharpness, 0x0002);
	}).min(lmin).max(lmax).step(lsd);

	// == Focus ==
	VI.getVideoSettingCamera(dev, VI.propFocus, lmin, lmax, lsd, lc, lf, ldv);
	focus = lc;
	mParams->addParam("focus", &focus).updateFn([this](){
		VI.setVideoSettingCamera(dev, VI.propFocus, focus, 0x0002);
	}).min(lmin).max(lmax).step(lsd);

	// == BacklightCompensation ==
	VI.getVideoSettingFilter(dev, VI.propBacklightCompensation, lmin, lmax, lsd, lc, lf, ldv);
	backlightCompensation = lc;
	mParams->addParam("backlightCompensation", &backlightCompensation).updateFn([this](){
		VI.setVideoSettingFilter(dev, VI.propBacklightCompensation, backlightCompensation, 0x0002);
	}).min(lmin).max(lmax).step(lsd);
	
	// == WhiteBalance ==
	VI.getVideoSettingFilter(dev, VI.propWhiteBalance, lmin, lmax, lsd, lc, lf, ldv);
	whiteBalance = lc;
	mParams->addParam("whiteBalance", &whiteBalance).updateFn([this](){
		VI.setVideoSettingFilter(dev, VI.propWhiteBalance, whiteBalance, 0x0002);
	}).min(lmin).max(lmax).step(lsd);

	mParams->addSeparator();
	mParams->addButton("save setting", std::bind(&WebCamApp::saveCamParamsCB, this));
	mParams->addButton("load setting", std::bind(&WebCamApp::loadCamParamsCB, this));
}

void WebCamApp::update()
{
	if (VI.isFrameNew(dev)) {		
		VI.getPixels(dev, mSurface.getData(), true, true);
		mTexture = gl::Texture::create(mSurface);
	}
}

void WebCamApp::draw()
{
	gl::clear(Color(0.0f, 0.0f, 0.0f));
	gl::setMatricesWindow(getWindowWidth(), getWindowHeight());

	if (mTexture) {
		gl::draw(mTexture);
	}

	mParams->draw();
}

CINDER_APP_NATIVE(WebCamApp, RendererGl)
