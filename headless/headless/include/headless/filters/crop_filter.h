#include <headless/filter_base.h>

class crop_filter : public filter_base
{
public:
	crop_filter();
	~crop_filter() override;

	std::unique_ptr<filter_base> clone() const override;
	const std::string type() const override { return "crop-filter"; };
	const bool apply(cv::Mat& mat) const override;
	const bool load_json(const nlohmann::json& filter) override;
	const nlohmann::json to_json() const override;

private:
	int size_x;
	int size_y;
};