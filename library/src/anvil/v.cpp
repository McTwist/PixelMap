#include "anvil/v.hpp"

#include "anvil/version.hpp"

anvil::V::V(Chunk & _chunk)
	: chunk(_chunk)
{
}

bool anvil::V::visit(const NBT::Value & value)
{
	(void)value;
	return false;
}

bool anvil::V::visit(const NBT::Tag & tag)
{
	if (tag.isName(""))
		return false;
	else if (tag.isName("Level"))
		return false;
	// Before DataVersion
	else if (chunk.getDataVersion()	== 0 && tag.isName("V"))
		chunk.setDataVersion(tag.get<int8_t>());
	// After DataVersion
	else if (tag.isName("DataVersion"))
	{
		chunk.setDataVersion(tag);
		if (chunk.getDataVersion() < DATA_VERSION_1_13)
			chunk.setPaletteType(PT_BLOCKID);
		else
			chunk.setPaletteType(PT_NAMESPACEID);
	}
	return true;
}
