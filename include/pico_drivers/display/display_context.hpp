#ifndef PICO_DRIVERS_DISPLAY_DISPLAY_CONTEXT_HPP_
#define PICO_DRIVERS_DISPLAY_DISPLAY_CONTEXT_HPP_

#include <cstdint>
#include <memory>
#include <utility>

#include <pico_drivers/display/frame_buffer.hpp>
#include <pico_drivers/display/gfx_font.hpp>
#include <pico_drivers/display/frame_streamer.hpp>
#include <pico_drivers/display/shape_rasterizer.hpp>
#include <pico_drivers/display/text_renderer.hpp>

struct DisplayContext {
    FrameBuffer frame_buffer;
    ShapeRasterizer shape_rasterizer;
    TextRenderer text_renderer;
    FrameStreamer frame_streamer;

    DisplayContext(uint16_t width, uint16_t height, FrameStreamer streamer,
                   ShapeRasterizer::PixelWriter pixel_writer,
                   ShapeRasterizer::SpanWriter span_writer,
                   TextRenderer::PixelWriter text_writer, const GFXfont *font)
        : frame_buffer(static_cast<std::size_t>(width) * (height / 8u)),
          shape_rasterizer(width, height, std::move(pixel_writer), std::move(span_writer)),
          text_renderer(width, height, std::move(text_writer), font),
          frame_streamer(std::move(streamer)) {}
};

#endif // PICO_DRIVERS_DISPLAY_DISPLAY_CONTEXT_HPP_
