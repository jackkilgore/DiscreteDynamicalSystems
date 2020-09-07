#ifndef INCLUDE_AL_DISTRIBUTEDAPP_HPP
#define INCLUDE_AL_DISTRIBUTEDAPP_HPP

/* Keehong Youn, 2017, younkeehong@gmail.com
 * Andres Cabrera, 2018, 2019, mantaraya36@gmail.com
 */

#include <iostream>
#include <map>

#include "al/app/al_App.hpp"
#include "al/app/al_NodeConfiguration.hpp"
#include "al/app/al_OmniRendererDomain.hpp"
#include "al/io/al_Socket.hpp"
#include "al/io/al_Toml.hpp"
#include "al/scene/al_DistributedScene.hpp"
#include "al/scene/al_DynamicScene.hpp"

/*
 * MPI and cuttlebone are optional.
 */
#ifdef AL_BUILD_MPI
#include <mpi.h>
#include <unistd.h>
#endif

#ifdef AL_USE_CUTTLEBONE
#include "Cuttlebone/Cuttlebone.hpp"
#endif

namespace al {

class AudioControl {
 public:
  void registerAudioIO(AudioIO &io) {
    gain.registerChangeCallback([&io](float value) { io.gain(value); });
  }

  Parameter gain{"gain", "sound", 1.0, "alloapp", 0.0, 2.0};
};

/**
 * @brief DistributedApp class
 * @ingroup App
 */
class DistributedApp : public App, public NodeConfiguration {
 public:
  DistributedApp();

  void start() override;

  std::string name();

  void initialize();

  void registerDynamicScene(DynamicScene &scene);

  Graphics &graphics() override;
  Window &defaultWindow() override;
  Viewpoint &view() override;
  Pose &pose() override;
  Lens &lens() override;
  Nav &nav() override;

  std::shared_ptr<GLFWOpenGLOmniRendererDomain> omniRendering;
  std::map<std::string, std::string> additionalConfig;

 private:
  AudioControl mAudioControl;

  std::map<std::string, std::string> mRoleMap;
};

template <class TSharedState>
class DistributedAppWithState : public DistributedApp {
 public:
  DistributedAppWithState() : DistributedApp() {
    // State will be same memory for local, but will be synced on the network
    // for separate instances

    mOpenGLGraphicsDomain->removeSubDomain(simulationDomain());

    // Replace Simulation domain with state simulation domain
    mSimulationDomain =
        mOpenGLGraphicsDomain
            ->newSubDomain<StateDistributionDomain<TSharedState>>(true);

    if (rank == 0) {
      std::cout << "Running primary" << std::endl;
      auto sender =
          std::static_pointer_cast<StateDistributionDomain<TSharedState>>(
              mSimulationDomain)
              ->addStateSender("state");
      sender->configure(10101);
    } else {
      std::cout << "Running REPLICA" << std::endl;
      auto receiver =
          std::static_pointer_cast<StateDistributionDomain<TSharedState>>(
              mSimulationDomain)
              ->addStateReceiver("state");
      receiver->configure(10101);
      mSimulationDomain
          ->disableProcessingCallback();  // Replicas won't call onAnimate()
    }
  }

  TSharedState &state() {
    return std::static_pointer_cast<StateDistributionDomain<TSharedState>>(
               mSimulationDomain)
        ->state();
  }

  void setTitle(std::string title) { defaultWindow().title(title); }

 private:
};

// struct DefaultStateDistributedApp {
//	Pose pose;
//};

// template<class TSharedState = DefaultStateDistributedApp>
// class DistributedApp: public OmniRenderer,
//           public AudioApp,
////           public FlowAppParameters,
//           public osc::PacketHandler
//{
//  Nav mNav; // is a Pose itself and also handles manipulation of pose
//  Viewpoint mView {mNav.transformed()};  // Pose with Lens and acts as camera
//  NavInputControl mNavControl {mNav}; // interaction with keyboard and mouse

// public:

//  typedef enum {
//      ROLE_NONE = 0,
//      ROLE_SIMULATOR = 1 << 1,
//      ROLE_RENDERER = 1 << 2,
//      ROLE_AUDIO = 1 << 3,
//      ROLE_CONTROL = 1 << 4,
//      ROLE_INTERFACE = 1 << 5, // For interface server
//      ROLE_DESKTOP = 1 << 6, // Application runs as single desktop app
//      ROLE_DESKTOP_REPLICA = 1 << 7, // Application runs as single desktop app
//      ROLE_USER  = 1 << 8 // User defined roles can add from here through
//      bitshifting
//  } Role;

//  typedef enum {
//    CAP_AUDIO = 1 << 1,
//    CAP_GRAPHICS = 1 << 2,
//    CAP_SIMULATE_SYNC = 1 << 3,
//    CAP_SIMULATE_ASYNC = 1 << 4,
//    CAP_PARAMETER_SERVER = 1 << 5,
//    CAP_FLOW_CLIENT = 1 << 6,
//    CAP_CUTTLEBONE = 1 << 7,
//    CAP_OPENVR = 1 << 7,
//    CAP_USER = 1 << 10 // User defined roles can add from here through
//    bitshifting
//  } Capability;

//  DistributedApp() { }

//  ~DistributedApp() {
//#ifdef AL_USE_CUTTLEBONE
//      if (mMaker) {
//          mMaker->stop();
//      } else if (mTaker){
//          mTaker->stop();
//      }
//#endif
//  }

//  std::string dataRoot() { return mGlobalDataRootPath;}

//  Role role() { return mRole;}

//  int rank() { return mRank; }
//  int group() { return mGroup; }

//  bool hasRole(Role role) { return mRole & role;}

//  std::string roleName() {
//    std::string name;
//    if (mRole & ROLE_SIMULATOR) {
//      name += "simulator+";
//    }
//    if (mRole & ROLE_RENDERER) {
//      name += "renderer+";
//    }
//    if (mRole & ROLE_AUDIO) {
//      name += "audio+";
//    }
//    if (mRole & ROLE_CONTROL) {
//      name += "control+";
//    }
//    if (mRole & ROLE_INTERFACE) {
//      name += "interface+";
//    }
//    if (mRole & ROLE_DESKTOP) {
//      name += "desktop+";
//    }
//    if (mRole & ROLE_DESKTOP_REPLICA) {
//      name += "replica+";
//    }
//    if (name.size() == 0 && mRole != ROLE_NONE) {
//      // TODO show user role index (amount of bitshift from ROLE_USER)
//      name += "[user role]+";
//    }
//    if (name.size() > 0) {
//      name = name.substr(0, name.size() -1);
//    } else {
//      name = "none";
//    }
//    return name;
//  }

//  void print(std::ostream& stream = std::cout) {
//#ifdef AL_BUILD_MPI
//      std::cout << "Processor: " << processor_name
//                << " Rank: " << world_rank
//                << " Of: " << world_size << std::endl;
//      std::cout << name() << ":" << world_rank << " set role to " <<
//      roleName() << std::endl;
//#else
//      stream << "DistributedApp: Not using MPI. Role: " << roleName() <<
//      std::endl;
//#endif
//  }

//  virtual void initAudio (double audioRate, int audioBlockSize,
//                          int audioOutputs, int audioInputs,
//                          int device = -1) override {
//    if (hasRole(ROLE_AUDIO) || hasRole(ROLE_DESKTOP) ||
//    hasRole(ROLE_DESKTOP_REPLICA)) {
//      AudioApp::initAudio(audioRate, audioBlockSize, audioOutputs,
//      audioInputs, device);
//    } else {
//      std::cout << "Audio disabled on this node.";
//    }
//  }

//  virtual void initAudio (AudioIOConfig config = OUT_ONLY) override {
//    if (hasRole(ROLE_AUDIO) || hasRole(ROLE_DESKTOP) ||
//    hasRole(ROLE_DESKTOP_REPLICA)) {
//      AudioApp::initAudio(config);
//    } else {
//      std::cout << "Audio disabled on this node.";
//    }
//  }

//  Viewpoint& view() { return mView; }
//  const Viewpoint& view() const { return mView; }

//  // Nav& nav() override { return mNav; }
//  Nav& nav() { return mNav; }
//  const Nav& nav() const { return mNav; }

//  Pose& pose() override { return mNav; }
//  const Pose& pose() const override { return mNav; }

//  NavInputControl& navControl() { return mNavControl; }
//  NavInputControl const& navControl() const { return mNavControl; }

//  Lens& lens() override { return mView.lens(); }
//  Lens const& lens() const override { return mView.lens(); }

//  // overrides (WindowApp & Omnirenderer)'s start to also initiate AudioApp
//  and etc. void start() override;

//  // interface from WindowApp
//  // users override these
//  void onInit() override {}
//  void onCreate() override {}
//  void onAnimate(double dt) override {(void) dt;}
//  void onDraw (Graphics& g) override {(void) g;}
//  void onExit() override {}
////  void onKeyDown(Keyboard const& k) override {(void) k;}
////  void onKeyUp(Keyboard const& k) override {(void) k;}
////  void onMouseDown(Mouse const& m) override {(void) m;}
////  void onMouseUp(Mouse const& m) override {(void) m;}
////  void onMouseDrag(Mouse const& m) override {(void) m;}
////  void onMouseMove(Mouse const& m) override {(void) m;}
//  void onResize(int w, int h) override {(void) w;(void) h;}
//  void onVisibility(bool v) override {(void) v;}

//  /// Override to compute updates to shared state
//  /// Currently run before calls to onAnimate()
//  /// i.e. this is synchronous to drawing and runs at
//  /// frame rate
//  virtual void simulate(double dt) { (void) dt; }

//  // extra functionalities to be handled
//  virtual void preOnCreate();
//  virtual void preOnAnimate(double dt);
//  virtual void preOnDraw();
//  virtual void postOnDraw();
//  virtual void postOnExit();

//  // PacketHandler
//  void onMessage(osc::Message& m) override { (void) m; }

//  /**
//   * @brief get current shared state
//   * @return reference to shared state.
//   *
//   * State should only be modified if hasRole(ROLE_SIMULATOR)
//   * Otherwise any changes made to state will not propagate.
//   */
//  TSharedState &state() { return mState;}
//  /**
//   * @brief returns the number of states received
//   *
//   * The number of states is updated prior to onAnimate() and onDraw()
//   * so it only really makes sense to check this within these two
//   * functions.
//   */
//  int newStates() { return mQueuedStates; }

//  void syncrhonize() {
//#ifdef AL_BUILD_MPI
//      MPI_Barrier(MPI_COMM_WORLD); // Wait for everybody
//#endif
//  }

//  void log(std::string logText) {
//      std::cout << name() << ":" << logText << std::endl;
//  }

//  ParameterServer &parameterServer() /*override*/ { return *mParameterServer;
//  }

////  static bool shouldRunDistributed() {
////    TomlLoader appConfig("distributed_app.toml");

////    for (auto key: *appConfig.root->get_table("nodes")) {
////      std::cout << key.first << " -- " << *key.second << std::endl;
////      if (al_get_hostname() == key.first) {
////        return true;
////      }
////    }

////    return false;
////  }

//  void registerDynamicScene(DynamicScene &scene) {
//    if (dynamic_cast<DistributedScene *>(&scene)) {
//      // If distributed scene, connect according to this app's role
//      DistributedScene *s =  dynamic_cast<DistributedScene *>(&scene);
//      if (isPrimary()) {
//       s->registerNotifier(parameterServer());
//      } else {
//        parameterServer().registerOSCConsumer(
//              s, s->name());
//      }
//    }

//    scene.prepare(audioIO());
//  }
// private:

//#ifdef AL_BUILD_MPI
//  // MPI data
//  int world_size;
//  int world_rank;
//  char processor_name[MPI_MAX_PROCESSOR_NAME];
//  int name_len;
//#endif
//  bool mRunDistributed {false};
//  Role mRole {ROLE_DESKTOP};
//  int mRank {0};
//  int mGroup {0};
//  std::string mGlobalDataRootPath;

//  std::map<std::string, Role> mRoleMap;

//  TSharedState mState;
//  int mQueuedStates {0};
//#ifdef AL_USE_CUTTLEBONE
//  std::unique_ptr<cuttlebone::Maker<TSharedState>> mMaker;
//  std::unique_ptr<cuttlebone::Taker<TSharedState>> mTaker;
//#endif
//  std::shared_ptr<ParameterServer> mParameterServer;

//  TomlLoader configLoader;

//};

//// ---------- IMPLEMENTATION
///---------------------------------------------------

// template<class TSharedState>
// inline void DistributedApp<TSharedState>::start() {

//    initializeWindowManager();
////  glfw::init(is_verbose);
//  initialize();

//  std::cout << name() << ":" << roleName()  << " before onInit" << std::endl;
//  onInit();

//  // must do before Window::create, overrides user given window diemnsions
//  check_if_in_sphere_and_setup_window_dimensions();

//  Window::create(is_verbose);
//  preOnCreate();
//  //  std::cout << name() << ":" << roleName()  << " before onCreate" <<
//  std::endl; onCreate();

//  if(hasRole(ROLE_AUDIO) || hasRole(ROLE_DESKTOP) ||
//  hasRole(ROLE_DESKTOP_REPLICA)) {
//    AudioApp::beginAudio(); // only begins if `initAudio` was called before
//  }

//  //  std::cout << name() << ":" << roleName() << " before init flow" <<
//  std::endl;

//  // FIXME put back FlowApp
//  //  if (role() & ROLE_SIMULATOR || role() & ROLE_DESKTOP) initFlowApp(true);
////  else initFlowApp(false);

//  if (mParameterServer) {
//    mParameterServer->registerOSCListener(this); // Have the parameter server
//    pass unhandled messages to this app's onMessage virtual function std::cout
//    << "Registered parameter server with Distributed App network socket"
//    <<std::endl;
//  }

//  FPS::startFPS(); // WindowApp (FPS)

//  while (!WindowApp::shouldQuit()) {
//    // to quit, call WindowApp::quit() or click close button of window,
//    // or press ctrl + q
//    if (hasRole(ROLE_DESKTOP) || hasRole(ROLE_SIMULATOR) ) {
//      simulate(dt_sec());
//      mQueuedStates = 1;
//#ifdef AL_USE_CUTTLEBONE
//      if (mMaker) {
//        mMaker->set(mState);
//      }
//#endif
//    } else {
//#ifdef AL_USE_CUTTLEBONE
//      if (mTaker) {
//        mQueuedStates = mTaker->get(mState);
//      }
//#else
//      if (!mRunDistributed) {
//      // You shouldn't get here if you are relying on cuttlebone for state
//      syncing
//        mQueuedStates = 1;
//      }
//#endif
//    }

//    preOnAnimate(dt_sec());
//    onAnimate(dt_sec());
//    bool forceOmni = false;
//    bool drawOmni = (hasRole(ROLE_RENDERER) && running_in_sphere_renderer) ||
//    forceOmni; if (drawOmni) {
//      draw_using_perprojection_capture();
//    }
//    else { // Not Omni
//      if (render_stereo) {
//        preOnDraw();
//        // check stereo window and do below to render in stereo when not in
//        sphere glDrawBuffer(GL_BACK_LEFT); mGraphics.eye(Graphics::LEFT_EYE);
//        onDraw(mGraphics);
//        postOnDraw();

//        preOnDraw();
//        glDrawBuffer(GL_BACK_RIGHT);
//        mGraphics.eye(Graphics::RIGHT_EYE);
//        onDraw(mGraphics);
//        postOnDraw();
//        glDrawBuffer(GL_BACK_LEFT);
//        mGraphics.eye(Graphics::MONO_EYE);

//      } else {
//        preOnDraw();
//        onDraw(mGraphics);
//        postOnDraw();
//      }
//    }
//    Window::refresh();
//    FPS::tickFPS();
//  }

//  onExit(); // user defined
//  postOnExit();
//  if(hasRole(ROLE_AUDIO) || hasRole(ROLE_DESKTOP) ||
//  hasRole(ROLE_DESKTOP_REPLICA)) {
//    AudioApp::endAudio(); // AudioApp
//  }
//  Window::destroy();
//  terminateWindowManager();

//}

// template<class TSharedState>
// inline void DistributedApp<TSharedState>::preOnCreate() {
//  append(mNavControl);
//#ifdef AL_USE_CUTTLEBONE
//  if (role() & ROLE_SIMULATOR) {
//      std::string broadcastAddress = configLoader.gets("broadcastAddress");
//      mMaker =
//      std::make_unique<cuttlebone::Maker<TSharedState>>(broadcastAddress.c_str());
//      mMaker->start();
//  } else if (role() & ROLE_RENDERER){
//      mTaker = std::make_unique<cuttlebone::Taker<TSharedState>>();
//      mTaker->start();
//  }
//#endif

//  window_is_stereo_buffered = Window::displayMode() & Window::STEREO_BUF;
//  mGraphics.init();
//  if (role() & ROLE_RENDERER && running_in_sphere_renderer) {
//    load_perprojection_configuration();
//  }
//  if (role() & ROLE_RENDERER) {
//    cursorHide(true);
//  }
//}

// template<class TSharedState>
// inline void DistributedApp<TSharedState>::preOnAnimate(double dt) {
//    mNav.smooth(std::pow(0.0001, dt));
//    mNav.step(dt * fps());
//}

// template<class TSharedState>
// inline void DistributedApp<TSharedState>::preOnDraw() {
//    mGraphics.framebuffer(FBO::DEFAULT);
//    mGraphics.viewport(0, 0, fbWidth(), fbHeight());
//    mGraphics.resetMatrixStack();
//    mGraphics.camera(mView);
//    mGraphics.color(1, 1, 1);
//}

// template<class TSharedState>
// inline void DistributedApp<TSharedState>::postOnDraw() {
//  //
//}

// template<class TSharedState>
// inline void DistributedApp<TSharedState>::postOnExit() {
//  //
//}

}  // namespace al

#endif
