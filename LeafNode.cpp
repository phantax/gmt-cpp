#include "LeafNode.h"
#include "String_.h"
#include <iostream>
#include <stdexcept>

using std::cout;
using std::endl;
using std::vector;
using std::string;


/*
 * ___________________________________________________________________________
 */
TypeDescriptor LeafNode::desc_(
		DataUnit::typeDescriptor(), 0, "LeafNode");


/*
 * ___________________________________________________________________________
 */
LeafNode::LeafNode() : DataUnit(), buffer_() {
}


/*
 * ___________________________________________________________________________
 */
LeafNode::LeafNode(const BC& capacity) :
		DataUnit(), buffer_(capacity) {

    /* TODO: do we really need this? */
	this->dissector().setSize(capacity);
}


/*
 * ___________________________________________________________________________
 */
const TypeDescriptor& LeafNode::getTypeDescriptor() const {

	return typeDescriptor();
}


/*
 * ___________________________________________________________________________
 */
void LeafNode::printBody(const PrintOptions& options) const {

	bool first = true;

	String pref(options.prefix);
	pref.setWidth(50);

	BC pos = 0;
	BC len = buffer_.getLength();
	while (pos == 0 || pos < len) {
		String hex = buffer_.toHexString(true, pos, 8);
		if (options.from.isDef() && options.from > pos) {
			for (size_t i = 0; i < 8 && i < (options.from - pos).byte(); i++) {
				if (hex.length() > 3*i) {
					hex.at(3*i) = '-';
					hex.at(3*i + 1) = '-';

				}
			}
		}
		hex.setWidth(23);
		if (!first) {
			cout << endl << pref;
		}
		cout << "| " << String::makeBoldRed(hex) << " |";
		first = false;
		pos += 8;
	}

	DataUnit::printBody(options);
}


/*
 * ___________________________________________________________________________
 */
string LeafNode::getAsText() const {

	String text;
	text.appendFormat("%s(\"%s\", %s of %s) [ %s ] <%s>",
			this->getTypeName().c_str(), this->getRefName().c_str(),
			this->getLength().toString().c_str(),
			this->dissector().getSize().toString().c_str(),
			this->toHexString(true).c_str(),
			this->getInfo_().c_str());
	return text;
}


/*
 * ___________________________________________________________________________
 */
void LeafNode::dissectorBodyReset() {

	/* clear the buffer to accept new data */
	buffer_.clear();
}


/*
 * ___________________________________________________________________________
 */
bool LeafNode::dissectorBodyIsAccepting() const {

	BC remCap = buffer_.getRemainingCapacity();
	return remCap.isUndef() || remCap > 0;
}


/*
 * ___________________________________________________________________________
 */
BC LeafNode::dissectorBodyDissect(StreamReader& reader) {

	BC len = buffer_.consumeStream(reader);

	if (buffer_.getCapacity() == buffer_.getLength()) {
		/* trigger decoding once raw data buffer has been filled */
		this->decode_(buffer_);
	}

	return len;
}


/*
 * ___________________________________________________________________________
 */
bool LeafNode::encode() {

	return this->encode_();
}


/*
 * ___________________________________________________________________________
 */
bool LeafNode::getBit_(const BC& bc) const {

	return buffer_.getBit(bc);
}


/*
 * ___________________________________________________________________________
 */
uint8_t	LeafNode::getByte_(const BC& bc) const {

	return buffer_.getByte(bc);
}


/*
 * ___________________________________________________________________________
 */
BC LeafNode::copyTo_(BufferWriter& buffer) const {

	return buffer_.copyTo(buffer);
}


/*
 * ___________________________________________________________________________
 */
BC LeafNode::getLength_() const {

	return buffer_.getLength();
}


/*
 * ___________________________________________________________________________
 */
bool LeafNode::appendBit(bool bit) {

	return buffer_.appendBit(bit);
}


/*
 * ___________________________________________________________________________
 */
bool LeafNode::appendByte(uint8_t byte) {

	return buffer_.appendByte(byte);
}


/*
 * ___________________________________________________________________________
 */
bool LeafNode::setBit(const BC& bc, bool bit) {

	return buffer_.setBit(bc, bit);
}


/*
 * ___________________________________________________________________________
 */
bool LeafNode::setByte(const BC& bc, uint8_t byte) {

	return buffer_.setByte(bc, byte);
}


/*
 * ___________________________________________________________________________
 */
void LeafNode::clear() {

	buffer_.clear();
}


/*
 * ___________________________________________________________________________
 */
void LeafNode::truncate(const BC& length) {

	buffer_.truncate(length);
}


/*
 * ___________________________________________________________________________
 */
void LeafNode::clearBuffer() {

	buffer_.clear();
}


/*
 * ___________________________________________________________________________
 */
BC LeafNode::getCapacity() const {

	return buffer_.getCapacity();
}


/*
 * ___________________________________________________________________________
 */
LeafNode::~LeafNode() {
}
