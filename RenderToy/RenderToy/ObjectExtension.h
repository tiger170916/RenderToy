#pragma once

#include "Includes.h"

enum class ExtensionType
{
	EXTENSION_TYPE_LIGHT = 1,
};

/// <summary>
/// Base of all object extensions
/// </summary>
class ObjectExtension
{
protected:
	uint32_t m_uid;

	ExtensionType m_extensionType;

public:
	ObjectExtension(uint32_t uid);

	virtual ~ObjectExtension() = 0;

	inline const uint32_t GetUid() const { return m_uid; }

	inline const ExtensionType GetExtensionType() const { return m_extensionType; }
};