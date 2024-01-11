#include <headless/filter_base.h>

class moving_average_filter : public filter_base
{
public:
	moving_average_filter();
	~moving_average_filter() override;

	std::unique_ptr<filter_base> clone() const override;
	const std::string type() const override { return "moving-average-filter"; };
	const bool apply(cv::Mat& mat) const override;
	const bool load_json(const nlohmann::json& filter) override;
	const nlohmann::json to_json() const override;

private:
	int size_x;
	int size_y;
};