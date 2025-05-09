/*
nifly
C++ NIF library for the Gamebryo/NetImmerse File Format
See the included GPLv3 LICENSE file
*/

#include "BasicTypes.hpp"
#include "NifUtil.hpp"

#include <array>
#include <regex>

using namespace nifly;

static const std::string NIF_GAMEBRYO = "Gamebryo File Format";
static const std::string NIF_NETIMMERSE = "NetImmerse File Format";
static const std::string NIF_NDS = "NDSNIF....@....@....";
static const std::string NIF_VERSTRING = ", Version ";

NiVersion::NiVersion(NiFileVersion _file, uint32_t _user, uint32_t _stream)
	: user(_user)
	, stream(_stream) {
	SetFile(_file);
}

std::string NiVersion::GetVersionInfo() const {
	return vstr + "\nUser Version: " + std::to_string(user) + "\nStream Version: " + std::to_string(stream);
}

void NiVersion::SetFile(NiFileVersion fileVer) {
	std::vector<uint8_t> verArr = ToArray(fileVer);
	std::string verNum;

	if (fileVer > V3_1) {
		verNum = std::to_string(verArr[0]) + '.' + std::to_string(verArr[1]) + '.' + std::to_string(verArr[2])
				 + '.' + std::to_string(verArr[3]);
	}
	else {
		verNum = std::to_string(verArr[0]) + '.' + std::to_string(verArr[1]);
	}

	if (nds != 0)
		vstr = NIF_NDS;
	else if (fileVer < V10_0_0_0)
		vstr = NIF_NETIMMERSE;
	else
		vstr = NIF_GAMEBRYO;

	vstr += NIF_VERSTRING;
	vstr += verNum;

	file = fileVer;
}


void NiString::Read(NiIStream& stream, const int szSize) {
	std::unique_ptr<char[]> buf;

	if (szSize == 1) {
		uint8_t smSize = 0;
		stream >> smSize;

		buf = std::make_unique<char[]>(smSize + 1);
		stream.read(buf.get(), smSize);
		buf[smSize] = 0;
	}
	else if (szSize == 2) {
		uint16_t medSize = 0;
		stream >> medSize;

		buf = std::make_unique<char[]>(medSize + 1);
		stream.read(buf.get(), medSize);
		buf[medSize] = 0;
	}
	else if (szSize == 4) {
		uint32_t bigSize = 0;
		stream >> bigSize;

		buf = std::make_unique<char[]>(bigSize + 1);
		stream.read(buf.get(), bigSize);
		buf[bigSize] = 0;
	}
	else
		return;

	str = buf.get();
}

void NiString::Write(NiOStream& stream, const int szSize) {
	if (szSize == 1) {
		auto sz = uint8_t(str.length());
		str.resize(sz);

		if (nullOutput)
			sz += 1;

		stream << sz;
	}
	else if (szSize == 2) {
		auto sz = uint16_t(str.length());
		str.resize(sz);

		if (nullOutput)
			sz += 1;

		stream << sz;
	}
	else if (szSize == 4) {
		auto sz = uint32_t(str.length());
		str.resize(sz);

		if (nullOutput)
			sz += 1;

		stream << sz;
	}

	stream.write(str.c_str(), str.length());
	if (nullOutput)
		stream << uint8_t(0);
}


void NiStringRef::Read(NiIStream& stream) {
	if (stream.GetVersion().File() < V20_1_0_3) {
		std::array<char, 2048 + 1> buf{};

		uint32_t sz = 0;
		stream >> sz;

		if (sz < buf.size())
			stream.read(buf.data(), sz);
		else
			sz = static_cast<uint32_t>(buf.size() - 1);

		buf[sz] = 0;
		str = buf.data();
	}
	else {
		stream >> index;
		
		if (index != NIF_NPOS && index > NIF_STRING_INDEX_LIMIT)
			throw std::length_error("Read: String index is too high.");
	}
}

void NiStringRef::Write(NiOStream& stream) {
	if (stream.GetVersion().File() < V20_1_0_3) {
		auto sz = uint32_t(str.length());
		str.resize(sz);

		stream << sz;
		stream.write(str.c_str(), str.length());
	}
	else {
		if (index != NIF_NPOS && index > NIF_STRING_INDEX_LIMIT)
			throw std::length_error("Write: String index is too high.");

		stream << index;
	}
}


void NiHeader::Clear() {
	numBlockTypes = 0;
	numStrings = 0;
	numBlocks = 0;
	blocks = nullptr;
	blockTypes.clear();
	blockTypeIndices.clear();
	blockSizes.clear();
	strings.clear();
}

std::string NiHeader::GetCreatorInfo() const {
	return creator.get();
}

void NiHeader::SetCreatorInfo(const std::string& creatorInfo) {
	creator.get() = creatorInfo;
}

std::string NiHeader::GetExportInfo() const {
	std::string exportInfo = exportInfo1.get();

	if (exportInfo2.length() > 0) {
		exportInfo.append("\n");
		exportInfo.append(exportInfo2.get());
	}

	if (exportInfo3.length() > 0) {
		exportInfo.append("\n");
		exportInfo.append(exportInfo3.get());
	}

	return exportInfo;
}

void NiHeader::SetExportInfo(const std::string& exportInfo) {
	exportInfo1.clear();
	exportInfo2.clear();
	exportInfo3.clear();

	std::vector<NiString*> exportStrings(3);
	exportStrings[0] = &exportInfo1;
	exportStrings[1] = &exportInfo2;
	exportStrings[2] = &exportInfo3;

	auto it = exportStrings.begin();
	for (size_t i = 0; i < exportInfo.length() && it < exportStrings.end(); i += 254, ++it) {
		if (i + 254 <= exportInfo.length())
			(*it)->get() = exportInfo.substr(i, 254);
		else
			(*it)->get() = exportInfo.substr(i, exportInfo.length() - i);
	}
}

uint32_t NiHeader::GetBlockID(NiObject* block) const {
	auto it = find_if(*blocks, [&block](const auto& ptr) { return ptr.get() == block; });

	if (it != blocks->end())
		return static_cast<uint32_t>(std::distance(blocks->begin(), it));

	return NIF_NPOS;
}

void NiHeader::DeleteBlock(const uint32_t blockId) {
	if (blockId == NIF_NPOS)
		return;

	uint16_t blockTypeId = blockTypeIndices[blockId];
	int blockTypeRefCount = 0;
	for (uint16_t blockTypeIndice : blockTypeIndices)
		if (blockTypeIndice == blockTypeId)
			blockTypeRefCount++;

	if (blockTypeRefCount < 2) {
		blockTypes.erase(blockTypes.begin() + blockTypeId);
		numBlockTypes--;
		for (uint16_t& blockTypeIndice : blockTypeIndices)
			if (blockTypeIndice > blockTypeId)
				blockTypeIndice--;
	}

	blockTypeIndices.erase(blockTypeIndices.begin() + blockId);

	if (version.File() >= V20_2_0_5)
		blockSizes.erase(blockSizes.begin() + blockId);

	blocks->erase(blocks->begin() + blockId);
	numBlocks--;

	// Next tell all the blocks that the deletion happened
	for (auto& b : (*blocks))
		BlockDeleted(b.get(), blockId);
}

void NiHeader::DeleteBlock(const NiRef& blockRef) {
	DeleteBlock(blockRef.index);
}

void NiHeader::DeleteBlockByType(const std::string& blockTypeStr, const bool orphanedOnly) {
	uint16_t blockTypeId = 0;
	for (blockTypeId = 0; blockTypeId < numBlockTypes; blockTypeId++)
		if (blockTypes[blockTypeId].get() == blockTypeStr)
			break;

	if (blockTypeId == numBlockTypes)
		return;

	std::vector<int> indices;
	for (uint32_t i = 0; i < numBlocks; i++)
		if (blockTypeIndices[i] == blockTypeId)
			indices.push_back(i);

	for (uint32_t j = static_cast<uint32_t>(indices.size()) - 1; j != NIF_NPOS; j--)
		if (!orphanedOnly || !IsBlockReferenced(indices[j]))
			DeleteBlock(indices[j]);
}

uint32_t NiHeader::AddBlock(std::unique_ptr<NiObject> newBlock) {
	uint16_t btID = AddOrFindBlockTypeId(newBlock->GetBlockName());
	blockTypeIndices.push_back(btID);

	if (version.File() >= V20_2_0_5)
		blockSizes.push_back(0);

	blocks->emplace_back(std::move(newBlock));
	numBlocks++;
	return numBlocks - 1;
}

uint32_t NiHeader::ReplaceBlock(const uint32_t oldBlockId, std::unique_ptr<NiObject> newBlock) {
	if (oldBlockId == NIF_NPOS)
		return NIF_NPOS;

	uint16_t blockTypeId = blockTypeIndices[oldBlockId];
	int blockTypeRefCount = 0;
	for (uint16_t blockTypeIndice : blockTypeIndices)
		if (blockTypeIndice == blockTypeId)
			blockTypeRefCount++;

	if (blockTypeRefCount < 2) {
		blockTypes.erase(blockTypes.begin() + blockTypeId);
		numBlockTypes--;
		for (uint16_t& blockTypeIndice : blockTypeIndices)
			if (blockTypeIndice > blockTypeId)
				blockTypeIndice--;
	}

	uint16_t btID = AddOrFindBlockTypeId(newBlock->GetBlockName());
	blockTypeIndices[oldBlockId] = btID;

	if (version.File() >= V20_2_0_5)
		blockSizes[oldBlockId] = 0;

	(*blocks)[oldBlockId].swap(newBlock);
	return oldBlockId;
}

void NiHeader::SetBlockOrder(std::vector<uint32_t>& newOrder) {
	if (newOrder.size() != numBlocks)
		return;

	std::vector<uint16_t> newBlockTypeIndices(blockTypeIndices.size());
	std::vector<std::unique_ptr<NiObject>> newBlocks(blocks->size());

	for (uint32_t i = 0; i < numBlocks; i++) {
		newBlockTypeIndices[newOrder[i]] = blockTypeIndices[i];
		newBlocks[newOrder[i]] = std::move(blocks->at(i));
	}

	if (version.File() >= V20_2_0_5) {
		std::vector<uint32_t> newBlockSizes(blockSizes.size());

		for (uint32_t i = 0; i < numBlocks; i++)
			newBlockSizes[newOrder[i]] = blockSizes[i];

		blockSizes = std::move(newBlockSizes);
	}

	blockTypeIndices = std::move(newBlockTypeIndices);
	(*blocks) = std::move(newBlocks);

	for (auto& b : (*blocks)) {
		std::set<NiRef*> refs;
		b->GetChildRefs(refs);

		for (auto& r : refs) {
			if (!r->IsEmpty() && r->index < newOrder.size())
				r->index = newOrder[r->index];
		}

		std::set<NiRef*> ptrs;
		b->GetPtrs(ptrs);

		for (auto& p : ptrs) {
			if (!p->IsEmpty() && p->index < newOrder.size())
				p->index = newOrder[p->index];
		}
	}
}

bool NiHeader::IsBlockReferenced(const uint32_t blockId, bool includePtrs) {
	if (blockId == NIF_NPOS)
		return false;

	for (auto& block : (*blocks)) {
		std::set<NiRef*> refs;
		block->GetChildRefs(refs);

		if (includePtrs)
			block->GetPtrs(refs);

		for (auto& ref : refs)
			if (ref->index == blockId)
				return true;
	}

	return false;
}

int NiHeader::GetBlockRefCount(const uint32_t blockId, bool includePtrs) {
	if (blockId == NIF_NPOS)
		return 0;

	int refCount = 0;

	for (auto& block : (*blocks)) {
		std::set<NiRef*> refs;
		block->GetChildRefs(refs);

		if (includePtrs)
			block->GetPtrs(refs);

		for (auto& ref : refs)
			if (ref->index == blockId)
				refCount++;
	}

	return refCount;
}

uint16_t NiHeader::AddOrFindBlockTypeId(const std::string& blockTypeName) {
	NiString niStr;
	auto typeId = static_cast<uint16_t>(blockTypes.size());
	for (uint16_t i = 0; i < typeId; i++) {
		if (blockTypes[i].get() == blockTypeName) {
			typeId = i;
			break;
		}
	}

	// Shader block type not found, add it
	if (typeId == blockTypes.size()) {
		niStr.get() = blockTypeName;
		blockTypes.push_back(niStr);
		numBlockTypes++;
	}
	return typeId;
}

std::string NiHeader::GetBlockTypeStringById(const uint32_t blockId) const {
	if (blockId != NIF_NPOS && blockId < numBlocks) {
		uint16_t typeIndex = blockTypeIndices[blockId];
		if (typeIndex < numBlockTypes)
			return blockTypes[typeIndex].get();
	}

	return std::string();
}

uint16_t NiHeader::GetBlockTypeIndex(const uint32_t blockId) const {
	if (blockId != NIF_NPOS && blockId < numBlocks)
		return blockTypeIndices[blockId];

	return 0xFFFF;
}

uint32_t NiHeader::GetBlockSize(const uint32_t blockId) const {
	if (blockId < numBlocks && blockSizes.size() > blockId)
		return blockSizes[blockId];

	return NIF_NPOS;
}

std::streampos NiHeader::GetBlockSizeStreamPos() const {
	return blockSizePos;
}

void NiHeader::ResetBlockSizeStreamPos() {
	blockSizePos = std::streampos();
}

uint32_t NiHeader::GetStringCount() const {
	return static_cast<uint32_t>(strings.size());
}

uint32_t NiHeader::FindStringId(const std::string& str) const {
	for (uint32_t i = 0; i < numStrings; i++)
		if (strings[i].get() == str)
			return i;

	return NIF_NPOS;
}

uint32_t NiHeader::AddOrFindStringId(const std::string& str, const bool addEmpty) {
	for (uint32_t i = 0; i < numStrings; i++)
		if (strings[i].get() == str)
			return i;

	if (!addEmpty && str.empty())
		return NIF_NPOS;

	constexpr auto maxStringCount = std::numeric_limits<uint32_t>::max();
	if (strings.size() >= static_cast<size_t>(maxStringCount))
		return NIF_NPOS;

	NiString niStr(str);
	strings.push_back(std::move(niStr));
	numStrings++;

	return numStrings - 1;
}

std::string NiHeader::GetStringById(const uint32_t id) const {
	if (id != NIF_NPOS && id < numStrings)
		return strings[id].get();

	return std::string();
}

void NiHeader::SetStringById(const uint32_t id, const std::string& str) {
	if (id != NIF_NPOS && id < numStrings)
		strings[id].get() = str;
}

void NiHeader::ClearStrings() {
	strings.clear();
	numStrings = 0;
	maxStringLen = 0;
}

void NiHeader::UpdateMaxStringLength() {
	maxStringLen = 0;
	for (auto& s : strings) {
		auto len = static_cast<uint32_t>(s.length());
		if (maxStringLen < len)
			maxStringLen = len;
	}
}

void NiHeader::FillStringRefs() {
	if (version.File() < V20_1_0_1)
		return;

	for (auto& b : (*blocks)) {
		std::vector<NiStringRef*> stringRefs;
		b->GetStringRefs(stringRefs);

		for (auto& r : stringRefs) {
			uint32_t stringId = r->GetIndex();

			// Check if string index is overflowing
			if (stringId != NIF_NPOS && stringId >= numStrings) {
				stringId -= numStrings;
				r->SetIndex(stringId);
			}

			std::string str = GetStringById(stringId);
			r->get() = str;
		}
	}
}

void NiHeader::UpdateHeaderStrings(const bool hasUnknown) {
	if (!hasUnknown)
		ClearStrings();

	if (version.File() < V20_1_0_1)
		return;

	for (auto& b : (*blocks)) {
		std::vector<NiStringRef*> stringRefs;
		b->GetStringRefs(stringRefs);

		for (auto& r : stringRefs) {
			bool addEmpty = (r->GetIndex() != NIF_NPOS);
			int stringId = AddOrFindStringId(r->get(), addEmpty);
			r->SetIndex(stringId);
		}
	}

	UpdateMaxStringLength();
}

void NiHeader::BlockDeleted(NiObject* o, const uint32_t blockId) {
	std::set<NiRef*> refs;
	o->GetChildRefs(refs);
	o->GetPtrs(refs);

	for (auto& r : refs) {
		if (!r->IsEmpty()) {
			if (r->index == blockId)
				r->Clear();
			else if (r->index > blockId)
				r->index--;
		}
	}
}

void NiHeader::Get(NiIStream& stream) {
	std::array<char, 128> ver{};
	stream.getline(ver.data(), ver.size());

	bool isNetImmerse = std::strstr(ver.data(), NIF_NETIMMERSE.c_str()) != nullptr;
	bool isGamebryo = std::strstr(ver.data(), NIF_GAMEBRYO.c_str()) != nullptr;
	bool isNDS = std::strstr(ver.data(), NIF_NDS.c_str()) != nullptr;

	if (!isNetImmerse && !isGamebryo && !isNDS)
		return;

	NiFileVersion vfile = UNKNOWN;
	uint32_t vuser = 0;
	uint32_t vstream = 0;

	auto verStrPtr = std::strstr(ver.data(), NIF_VERSTRING.c_str());
	if (verStrPtr) {
		std::string verStr = verStrPtr + 10;
		std::regex reg("25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9]");
		std::smatch matches;

		std::array<uint8_t, 4> v{};
		int m = 0;
		while (std::regex_search(verStr, matches, reg) && m < 4) {
			v[m] = static_cast<uint8_t>(std::stoi(matches[0]));
			verStr = matches.suffix();
			m++;
		}

		vfile = NiVersion::ToFile(v[0], v[1], v[2], v[3]);
	}

	if (vfile > V3_1 && !isNDS) {
		stream >> vfile;
	}
	else if (isNDS) {
		uint32_t versionNDS = 0;
		stream >> versionNDS;
		version.SetNDS(versionNDS);
	}
	else {
		const int len = 128;

		copyright1.resize(len);
		stream.getline(copyright1.data(), copyright1.size());

		copyright2.resize(len);
		stream.getline(copyright2.data(), copyright2.size());

		copyright3.resize(len);
		stream.getline(copyright3.data(), copyright3.size());
	}

	version.SetFile(vfile);

	if (version.File() >= NiVersion::ToFile(20, 0, 0, 3))
		stream >> endian;
	else
		endian = ENDIAN_LITTLE;

	if (version.File() >= NiVersion::ToFile(10, 0, 1, 8)) {
		stream >> vuser;
		version.SetUser(vuser);
	}

	stream >> numBlocks;

	if (version.IsBethesda()) {
		stream >> vstream;
		version.SetStream(vstream);

		creator.Read(stream, 1);

		if (version.Stream() > 130)
			stream >> unkInt1;

		exportInfo1.Read(stream, 1);
		exportInfo2.Read(stream, 1);

		if (version.Stream() == 130)
			exportInfo3.Read(stream, 1);
	}
	else if (version.File() >= V30_0_0_2) {
		stream >> embedDataSize;
		embedData.resize(embedDataSize);
		for (uint32_t i = 0; i < embedDataSize; i++)
			stream >> embedData[i];
	}

	if (version.File() >= V5_0_0_1) {
		stream >> numBlockTypes;
		blockTypes.resize(numBlockTypes);
		for (uint32_t i = 0; i < numBlockTypes; i++)
			blockTypes[i].Read(stream, 4);

		blockTypeIndices.resize(numBlocks);
		for (uint32_t i = 0; i < numBlocks; i++)
			stream >> blockTypeIndices[i];
	}

	if (version.File() >= V20_2_0_5) {
		blockSizes.resize(numBlocks);
		for (uint32_t i = 0; i < numBlocks; i++)
			stream >> blockSizes[i];
	}

	if (version.File() >= V20_1_0_1) {
		stream >> numStrings;
		stream >> maxStringLen;

		strings.resize(numStrings);
		for (uint32_t i = 0; i < numStrings; i++)
			strings[i].Read(stream, 4);
	}

	if (version.File() >= NiVersion::ToFile(5, 0, 0, 6)) {
		stream >> numGroups;
		groupSizes.resize(numGroups);
		for (uint32_t i = 0; i < numGroups; i++)
			stream >> groupSizes[i];
	}

	valid = true;
}

void NiHeader::Put(NiOStream& stream) {
	std::string ver = version.String();
	stream.write(ver.data(), ver.size());

	// Newline to end header string
	stream << uint8_t(0x0A);

	bool isNDS = version.NDS() != 0;
	if (version.File() > V3_1 && !isNDS) {
		stream << version.File();
	}
	else if (isNDS) {
		stream << version.NDS();
	}
	else {
		stream.writeline(copyright1.data(), copyright1.size());
		stream.writeline(copyright2.data(), copyright2.size());
		stream.writeline(copyright3.data(), copyright3.size());
	}

	if (version.File() >= NiVersion::ToFile(20, 0, 0, 3))
		stream << endian;

	if (version.File() >= NiVersion::ToFile(10, 0, 1, 8))
		stream << version.User();

	stream << numBlocks;

	if (version.IsBethesda()) {
		stream << version.Stream();

		creator.SetNullOutput();
		creator.Write(stream, 1);

		if (version.Stream() > 130)
			stream << unkInt1;

		exportInfo1.SetNullOutput();
		exportInfo1.Write(stream, 1);

		exportInfo2.SetNullOutput();
		exportInfo2.Write(stream, 1);

		if (version.Stream() == 130) {
			exportInfo3.SetNullOutput();
			exportInfo3.Write(stream, 1);
		}
	}
	else if (version.File() >= V30_0_0_2) {
		stream << embedDataSize;
		for (uint32_t i = 0; i < embedDataSize; i++)
			stream << embedData[i];
	}

	if (version.File() >= V5_0_0_1) {
		stream << numBlockTypes;
		for (uint16_t i = 0; i < numBlockTypes; i++)
			blockTypes[i].Write(stream, 4);

		for (uint32_t i = 0; i < numBlocks; i++)
			stream << blockTypeIndices[i];
	}

	if (version.File() >= V20_2_0_5) {
		blockSizePos = stream.tellp();
		for (uint32_t i = 0; i < numBlocks; i++)
			stream << blockSizes[i];
	}

	if (version.File() >= V20_1_0_1) {
		stream << numStrings;
		stream << maxStringLen;
		for (uint32_t i = 0; i < numStrings; i++)
			strings[i].Write(stream, 4);
	}

	if (version.File() >= NiVersion::ToFile(5, 0, 0, 6)) {
		stream << numGroups;
		for (uint32_t i = 0; i < numGroups; i++)
			stream << groupSizes[i];
	}
}


NiUnknown::NiUnknown(NiIStream& stream, const uint32_t size) {
	data.resize(size);

	blockSize = size;
	Get(stream);
}

NiUnknown::NiUnknown(const uint32_t size) {
	data.resize(size);

	blockSize = size;
}

void NiUnknown::Sync(NiStreamReversible& stream) {
	if (data.empty())
		return;

	stream.Sync(&data[0], blockSize);
}
