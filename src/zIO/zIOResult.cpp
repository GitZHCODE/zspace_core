#include <zspace/zIO/zIOResult.h>

#include <utility>

namespace zSpace
{
	zIOResult::zIOResult()
		: success_(true)
	{
	}

	zIOResult::zIOResult(bool success, std::string message)
		: success_(success), message_(std::move(message))
	{
	}

	zIOResult::operator bool() const noexcept
	{
		return success_;
	}

	bool zIOResult::success() const noexcept
	{
		return success_;
	}

	const std::string& zIOResult::message() const noexcept
	{
		return message_;
	}

	zIOResult zIOResult::ok()
	{
		return {};
	}

	zIOResult zIOResult::error(std::string message)
	{
		return { false, std::move(message) };
	}
}
