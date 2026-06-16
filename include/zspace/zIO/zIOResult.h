#ifndef ZSPACE_IO_RESULT_H
#define ZSPACE_IO_RESULT_H

#pragma once

#include <zspace/export.h>

#include <string>

namespace zSpace
{
	class ZSPACE_IO zIOResult
	{
	public:
		zIOResult();
		zIOResult(bool success, std::string message);

		explicit operator bool() const noexcept;
		bool success() const noexcept;
		const std::string& message() const noexcept;

		static zIOResult ok();
		static zIOResult error(std::string message);

	private:
		bool success_;
		ZSPACE_SUPPRESS_DLL_INTERFACE
		std::string message_;
	};
}

#endif
