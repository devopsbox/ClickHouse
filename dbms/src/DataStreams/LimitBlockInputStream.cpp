#include <algorithm>

#include <DB/DataStreams/LimitBlockInputStream.h>


namespace DB
{

using Poco::SharedPtr;

LimitBlockInputStream::LimitBlockInputStream(SharedPtr<IBlockInputStream> input_, size_t limit_, size_t offset_)
	: input(input_), limit(limit_), offset(offset_), pos(0)
{
}


Block LimitBlockInputStream::read()
{
	Block res;
	size_t rows = 0;

	/// pos - сколько строк было прочитано, включая последний прочитанный блок

	if (pos >= offset + limit)
		return res;

	do
	{
		res = input->read();
		rows = res.rows();
		pos += rows;
	} while (pos <= offset);

	/// отдать целый блок
	if (pos >= offset + rows && pos <= offset + limit)
		return res;

	/// отдать кусок блока
	size_t start = std::max(0, static_cast<int>(offset) - static_cast<int>(pos) + static_cast<int>(rows));
	size_t length = std::min(static_cast<int>(limit), std::min(
		static_cast<int>(pos) - static_cast<int>(offset),
		static_cast<int>(limit) + static_cast<int>(offset) - static_cast<int>(pos) + static_cast<int>(rows)));

	for (size_t i = 0; i < res.columns(); ++i)
		res.getByPosition(i).column->cut(start, length);
	
	return res;
}

}

