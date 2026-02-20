#include <grpcpp/support/status.h>
#include "renderer/v1/renderer.pb.h"
#include "renderer/v1/renderer.grpc.pb.h"
#include "session/render_options.h"
#include "session/render_session.h"

#include <exception>

using api::proto::renderer::v1::RenderLayerRequest;
using api::proto::renderer::v1::RenderLayerResponse;
using api::proto::renderer::v1::RendererService;

class RendererServiceImpl final : public RendererService::Service {
    grpc::Status RenderLayer(grpc::ServerContext* context,
                             const RenderLayerRequest* request,
                             RenderLayerResponse* response) override
    {

        try {
                std::cout << "Rendering layer: " << request->layer_name() << std::endl;

            // Configure render options
            skwr::RenderOptions options;
            options.image_config.width = request->width();
            options.image_config.height = request->height();
            options.image_config.outfile = request->output_target_uri();

            options.integrator_config.samples_per_pixel = request->samples_per_pixel();
            options.integrator_config.num_threads = request->num_threads();
            options.integrator_config.max_depth = request->max_depth();

            skwr::RenderSession session;
            session.LoadScene(request->scene_file());

            session.SetOptions(options);
            session.Render();
            session.Save();

            // Set response fields
            response->set_success(true);
            response->set_output_uri(request->output_target_uri());

            return grpc::Status::OK;
        } catch (const std::exception& e) {
            // Log the error to the console
            std::cerr << "[Render Worker Error] Caught exception: " << e.what() << std::endl;

            // Tell Go coordinator what happened
            response->set_success(false);
            response->set_error_msg(e.what());

            return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
        } catch (...) {
            std::cerr << "[Render Worker Error] Unknown error occurred!" << std::endl;
            return grpc::Status(grpc::StatusCode::UNKNOWN, "An unknown error occurred");
        }
    }
};
