#include "User.h"

IMPLEMENT_SERIAL(CUser, CObject,1)
//IMPLEMENT_DYNAMIC(CUser, CObject)

CUser::CUser()
{
	m_registered = FALSE;
}


CUser::~CUser()
{
}

void CUser::Serialize(CArchive& archive)
{
	// call base class function first
	// base class is CObject in this case
	CObject::Serialize(archive);

	// now do the stuff for our specific class
	if (archive.IsStoring())
		archive << m_name << m_email << m_registered;
	else
		archive >> m_name >> m_email >> m_registered;
}
