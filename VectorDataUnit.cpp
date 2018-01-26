#include "VectorDataUnit.h"


/*
 * ___________________________________________________________________________
 */
TypeDescriptor VectorDataUnit::desc_(
		InternalNode::typeDescriptor(), 0, "VectorDataUnit");


/*
 * ___________________________________________________________________________
 */
VectorDataUnit::VectorDataUnit() : InternalNode(), template_(0) {
}


/*
 * ___________________________________________________________________________
 */
VectorDataUnit::VectorDataUnit(bool empty)
	: InternalNode(empty), template_(0) {
}


/*
 * ___________________________________________________________________________
 */
VectorDataUnit::VectorDataUnit(DataUnit* elementTemplate) :
		InternalNode(), template_(elementTemplate) {

}


/*
 * ___________________________________________________________________________
 */
const TypeDescriptor& VectorDataUnit::getTypeDescriptor() const {

	return typeDescriptor();
}


/*
 * ___________________________________________________________________________
 */
void VectorDataUnit::setElementTemplate(DataUnit* elementTemplate) {

	if (template_ != 0) {
		delete template_;
	}
	template_ = elementTemplate;
}


/*
 * ___________________________________________________________________________
 */
DataUnit* VectorDataUnit::getElementTemplate() const {

	return template_;
}


/*
 * ___________________________________________________________________________
 */
VectorDataUnit::~VectorDataUnit() {

	/* delete the template if one has been given */
	if (template_ != 0) {
		delete template_;
	}
}

