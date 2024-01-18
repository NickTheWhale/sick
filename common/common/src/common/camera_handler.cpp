#include "common/camera_handler.h"

#include "spdlog/spdlog.h"

camera::camera_handler::camera_handler()
{
}

camera::camera_handler::~camera_handler()
{
    if (_visionary_control)
        _visionary_control->stopAcquisition();
}

const bool camera::camera_handler::open(const std::string& ip, const uint16_t& port, const uint32_t& timeout_ms)
{
    _frame_grabber.reset(new visionary::FrameGrabber<visionary::VisionaryTMiniData>(ip, htons(port), timeout_ms));
    if (!_frame_grabber)
    {
        spdlog::get("camera")->error("Failed to create frame grabber");
        return false;
    }

    _data_handler.reset(new visionary::VisionaryTMiniData);
    if (!_data_handler)
    {
        spdlog::get("camera")->error("Failed to create frame data buffer");
        return false;
    }

    _visionary_control.reset(new visionary::VisionaryControl);
    if (!_visionary_control)
    {
        spdlog::get("camera")->error("Failed to create camera control channel");
        return false;
    }

    if (!_visionary_control->open(visionary::VisionaryControl::ProtocolType::COLA_2, ip, timeout_ms))
    {
        spdlog::get("camera")->error("Failed to open camera control channel");
        return false;
    }

    if (!_visionary_control->stopAcquisition())
    {
        spdlog::get("camera")->error("Failed to stop frame acquisition");
        return false;
    }

    // start continuous acquisition
    if (!_visionary_control->startAcquisition())
    {
        spdlog::get("camera")->error("Failed to start frame acquisition");
        return false;
    }

    spdlog::get("camera")->info("Camera opened");

    return true;
}

const bool camera::camera_handler::get_current_frame(frame::Frame& frame)
{
    if (!_frame_grabber)
        return false;

    if (!_frame_grabber->getCurrentFrame(_data_handler))
        return false;

    frame.data = _data_handler->getDistanceMap();
    frame.height = _data_handler->getHeight();
    frame.width = _data_handler->getWidth();
    frame.number = _data_handler->getFrameNum();
    frame.time_ms = _data_handler->getTimestampMS();

    return true;
}

const bool camera::camera_handler::get_next_frame(frame::Frame& frame, const uint64_t timeout_ms)
{
    if (!_frame_grabber)
        return false;

    if (!_frame_grabber->getNextFrame(_data_handler, timeout_ms))
        return false;

    frame.data = _data_handler->getDistanceMap();
    frame.height = _data_handler->getHeight();
    frame.width = _data_handler->getWidth();
    frame.number = _data_handler->getFrameNum();
    frame.time_ms = _data_handler->getTimestampMS();

    return true;
}
