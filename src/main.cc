#include "wrappers/mediaio.h"
#include "wrappers/sdk.h"

#include "utils/commands_handler.h"

#include <iostream>
#include <vector>

using namespace dolbyio::comms::sample;

#if defined(__linux__)
#include "linux/daemonize.h"

#include <execinfo.h>
#include <signal.h>

// Global unique_ptr for linux daemonization, because I was too lazy
// to read sigaction and setting context for handler ;)
std::unique_ptr<daemonize> daemonize_ptr{nullptr};

void signal_handler(int sig) {
  if (sig == SIGTERM && daemonize_ptr)
    daemonize_ptr->unblock_indefinte_wait();
}
#endif

dolbyio::comms::async_result<void> apply_spatial_audio_configuration(
    sdk_wrapper* sdk) {
  auto params = sdk->get_params().conf;
  // Set default spatial environment
  dolbyio::comms::spatial_position right{1, 0, 0};
  dolbyio::comms::spatial_position up{0, 1, 0};
  dolbyio::comms::spatial_position forward{0, 0, -1};
  dolbyio::comms::spatial_scale scale{1, 1, 1};
  dolbyio::comms::spatial_audio_batch_update batch_update;
  batch_update.set_spatial_environment(scale, forward, up, right);
  batch_update.set_spatial_direction(params.initial_spatial_direction);

  dolbyio::comms::spatial_position position{params.initial_spatial_position};
  batch_update.set_spatial_position(sdk->session_info().participant_id.value(),
                                    std::move(position));
  return sdk->set_spatial_configuration(std::move(batch_update));
}

int main(int argc, char** argv) {
  commands_handler command_handler{};
#if defined(__linux__)
  signal(SIGTERM, signal_handler);
#else
  volatile bool quit = false;
  command_handler.add_interactive_command("q", "exit", [&quit]() { quit = true; });
#endif

  std::unique_ptr<dolbyio::comms::sdk> sdk{};
  auto sdk_wrap = std::make_shared<sdk_wrapper>();
  auto media_io_wrap =
      std::make_shared<media_io_wrapper>(sdk_wrap->get_params());

  try {
    command_handler.add_interactor(sdk_wrap);
    command_handler.add_interactor(media_io_wrap);

    command_handler.parse_command_line(argc, argv);

#if defined(__linux__)
    try {
      daemonize_ptr = std::make_unique<daemonize>(sdk_wrap->get_params().log_dir);
    } catch (daemon::failure_exception& ex) {
      exit(EXIT_FAILURE);
    } catch (daemon::parent_process_exception& ex) {
      exit(EXIT_SUCCESS);
    }
#endif

    // Create the SDK passing in the token and a refresh token callback
    sdk = dolbyio::comms::sdk::create(sdk_wrap->get_params().access_token,
                                      [](auto&&) {});
    command_handler.set_sdk(sdk.get());

    auto sdk_params = sdk_wrap->get_params();
    if (sdk_params.conf.nonlistener_join &&
        sdk_params.conf.spatial !=
            dolbyio::comms::spatial_audio_style::disabled) {
    }

    media_io_wrap->initialize_injection();

    {
      // Using promise/future to make the thread wait till this entire
      // asynchronous chain is completed. But like in general you could do
      // without just async it all.
      auto promise = std::make_shared<std::promise<void>>();
      auto future = promise->get_future();
      sdk_wrap->open_session()
          .then([sdk_wrap]() {
            return sdk_wrap->create_and_or_join_conference();
          })
          .then([sdk_wrap]() {
            return apply_spatial_audio_configuration(sdk_wrap.get());
          })
          .then([sdk_wrap]() { return sdk_wrap->set_audio_processing(); })
          .then([sdk_wrap, media_io_wrap, promise]() {
            auto media = sdk_wrap->get_params().conf;
            media_io_wrap->set_initial_capture(media.join_with_audio(),
                                               media.join_with_video());
            promise->set_value();
          })
          .on_error(
              [promise](auto&& ex) { promise->set_exception(std::move(ex)); });
      future.get();
    }

#if defined(__linux__)
    daemonize_ptr->wait_indefinitely();
#else
    while (!quit) {
      command_handler.print_interactive_options();
      std::string command;
      std::cin >> command;
      command_handler.handle_interactive_command(command);
    }
#endif
    {
      // Using promise/future to make the thread wait till this entire
      // asynchronous chain is completed. But like in general you could do
      // without just async it all.
      auto promise = std::make_shared<std::promise<void>>();
      auto future = promise->get_future();
      sdk_wrap->leave_conference()
          .then([sdk_wrap]() { return sdk_wrap->close_session(); })
          .then([promise]() { promise->set_value(); })
          .on_error(
              [promise](auto&& ex) { promise->set_exception(std::move(ex)); });
      future.get();
    }

  } catch (const std::exception& ex) {
    std::cout << "Something went wrong: " << ex.what() << std::endl;
  }
  command_handler.set_sdk(nullptr);
  return 0;
}