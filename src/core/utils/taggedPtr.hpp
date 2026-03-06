#pragma once
#include<cstddef>
#include<cstdint>


struct TaggedPtr {
	void* ptr;
	uint64_t  tag;


	TaggedPtr() : ptr{ nullptr }, tag{0} {}
	TaggedPtr(void* p, uint64_t tag) : ptr{ p }, tag{tag} {}

	bool operator==(const TaggedPtr& other) const {
		return (ptr == other.ptr) && (tag == other.tag);
	}

	bool operator!=(const TaggedPtr& other) const {
		return (ptr != other.ptr) || (tag != other.tag);

	}

};
