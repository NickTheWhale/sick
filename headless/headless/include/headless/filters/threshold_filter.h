#include <headless/filter_base.h>

class threshold_filter : public filter_base
{
public:
	threshold_filter();
	~threshold_filter() override;

	std::unique_ptr<filter_base> clone() const override;
	const std::string type() const override { return "threshold-filter"; };
	const bool apply(cv::Mat& mat) const override;
	const bool load_json(const nlohmann::json& filter) override;
	const nlohmann::json to_json() const override;

private:
	int size_x;
	int size_y;
};