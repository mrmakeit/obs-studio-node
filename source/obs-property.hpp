/******************************************************************************
    Copyright (C) 2016-2019 by Streamlabs (General Workings Inc)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#pragma once
#include <inttypes.h>
#include <list>
#include <memory>
#include <vector>

namespace obs
{
	struct Property
	{
		enum class Type : uint8_t
		{
			Invalid,
			Boolean,
			Integer,
			Float,
			Text,
			Path,
			List,
			Color,
			Button,
			Font,
			EditableList,
			FrameRate,
		};

		std::string name;
		std::string description;
		std::string long_description;
		bool        enabled;
		bool        visible;

		virtual ~Property(){};

		static std::shared_ptr<Property> deserialize(std::vector<char> const& buf);

		virtual obs::Property::Type type();
		virtual size_t              size();
		virtual bool                serialize(std::vector<char>& buf);

		protected:
		virtual bool read(std::vector<char> const& buf);
	};

	struct BooleanProperty : Property
	{
		virtual ~BooleanProperty(){};

		virtual obs::Property::Type type() override;
		virtual size_t              size() override;
		virtual bool                serialize(std::vector<char>& buf) override;

		protected:
		virtual bool read(std::vector<char> const& buf) override;
	};

	struct NumberProperty : Property
	{
		enum class NumberType : uint8_t
		{
			Scroller,
			Slider,
		};
		NumberType field_type;
		virtual ~NumberProperty(){};

		virtual size_t size() override;
		virtual bool   serialize(std::vector<char>& buf) override;

		protected:
		virtual bool read(std::vector<char> const& buf) override;
	};

	struct IntegerProperty : NumberProperty
	{
		int64_t minimum;
		int64_t maximum;
		int64_t step;

		virtual ~IntegerProperty(){};

		virtual obs::Property::Type type() override;
		virtual size_t              size() override;
		virtual bool                serialize(std::vector<char>& buf) override;

		protected:
		virtual bool read(std::vector<char> const& buf) override;
	};

	struct FloatProperty : NumberProperty
	{
		double_t minimum;
		double_t maximum;
		double_t step;

		virtual ~FloatProperty(){};

		virtual obs::Property::Type type() override;
		virtual size_t              size() override;
		virtual bool                serialize(std::vector<char>& buf) override;

		protected:
		virtual bool read(std::vector<char> const& buf) override;
	};

	struct TextProperty : Property
	{
		enum class TextType : uint8_t
		{
			Default,
			Password,
			MultiLine,
		};
		TextType field_type;

		virtual ~TextProperty(){};

		virtual obs::Property::Type type() override;
		virtual size_t              size() override;
		virtual bool                serialize(std::vector<char>& buf) override;

		protected:
		virtual bool read(std::vector<char> const& buf) override;
	};

	struct PathProperty : Property
	{
		enum class PathType
		{
			File,
			SaveFile,
			Directory,
		};
		PathType    field_type;
		std::string filter;
		std::string default_path;

		virtual ~PathProperty(){};

		virtual obs::Property::Type type() override;
		virtual size_t              size() override;
		virtual bool                serialize(std::vector<char>& buf) override;

		protected:
		virtual bool read(std::vector<char> const& buf) override;
	};

	struct ListProperty : Property
	{
		enum class ListType : uint8_t
		{
			Invalid,
			Editable,
			List,
		};
		ListType field_type;

		enum class Format : uint8_t
		{
			Invalid,
			Integer,
			Float,
			String,
		};
		Format format;

		struct Item
		{
			std::string name;
			bool        enabled;

			int64_t     value_int;
			double_t    value_float;
			std::string value_string;
		};
		std::list<Item> items;

		virtual ~ListProperty(){};

		virtual obs::Property::Type type() override;
		virtual size_t              size() override;
		virtual bool                serialize(std::vector<char>& buf) override;

		protected:
		virtual bool read(std::vector<char> const& buf) override;
	};

	struct ColorProperty : Property
	{
		virtual ~ColorProperty(){};

		virtual obs::Property::Type type() override;
		virtual size_t              size() override;
		virtual bool                serialize(std::vector<char>& buf) override;

		protected:
		virtual bool read(std::vector<char> const& buf) override;
	};

	struct ButtonProperty : Property
	{
		virtual ~ButtonProperty(){};

		virtual obs::Property::Type type() override;
		virtual size_t              size() override;
		virtual bool                serialize(std::vector<char>& buf) override;

		protected:
		virtual bool read(std::vector<char> const& buf) override;
	};

	struct FontProperty : Property
	{
		virtual ~FontProperty(){};

		virtual obs::Property::Type type() override;
		virtual size_t              size() override;
		virtual bool                serialize(std::vector<char>& buf) override;

		protected:
		virtual bool read(std::vector<char> const& buf) override;
	};

	struct EditableListProperty : Property
	{
		enum class ListType
		{
			Strings,
			Files,
			FilesAndURLs,
		};
		ListType    field_type;
		std::string filter;
		std::string default_path;

		virtual ~EditableListProperty(){};

		virtual obs::Property::Type type() override;
		virtual size_t              size() override;
		virtual bool                serialize(std::vector<char>& buf) override;

		protected:
		virtual bool read(std::vector<char> const& buf) override;
	};

	struct FrameRateProperty : Property
	{
		struct Range
		{
			std::pair<uint32_t, uint32_t> minimum;
			std::pair<uint32_t, uint32_t> maximum;
		};
		std::list<Range> ranges;

		struct Option
		{
			std::string name;
			std::string description;
		};
		std::list<Option> options;

		virtual ~FrameRateProperty(){};

		virtual obs::Property::Type type() override;
		virtual size_t              size() override;
		virtual bool                serialize(std::vector<char>& buf) override;

		protected:
		virtual bool read(std::vector<char> const& buf) override;
	};

} // namespace obs
