#include <gui/filter_base.h>

class stack_blur_filter : public filter_base
{
public:
	stack_blur_filter();
	~stack_blur_filter() override;

	std::unique_ptr<filter_base> clone() const override;
	const std::string type() const override { return "stack-blur-filter"; };
	const bool apply(cv::Mat& mat) const override;
	const bool from_json(const nlohmann::json& filter) override;
	const nlohmann::json to_json() const override;

private:
	int size_x;
	int size_y;
};