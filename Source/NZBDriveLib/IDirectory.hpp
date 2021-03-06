/*
    Copyright (c) 2017, Ole Stauning
    All rights reserved.

    Use of this source code is governed by a GPLv3-style license that can be found
    in the LICENSE file.
*/

#ifndef IDIRECTORY_H
#define IDIRECTORY_H

#include "ContentType.hpp"
#include "IFile.hpp"
#include <memory>
#include <vector>
#include <filesystem>

namespace ByteFountain
{
	class IDirectory
	{
	public:
		virtual ~IDirectory() {};

		virtual std::shared_ptr<IDirectory> GetDirectory(const std::filesystem::path& p) =0;
		virtual std::shared_ptr<IFile> GetFile(const std::filesystem::path& p) = 0;
		virtual std::vector< std::pair<std::filesystem::path, ContentType> > GetContent() = 0;
		virtual bool Exists(const std::filesystem::path& p) = 0;
		virtual void EnumFiles(std::function<void(const std::filesystem::path& path, std::shared_ptr<IFile> file)> callback,
			const std::filesystem::path& p = std::filesystem::path()) = 0;
		virtual uint_fast64_t GetTotalNumberOfBytes() const = 0;
	};
}

#endif
