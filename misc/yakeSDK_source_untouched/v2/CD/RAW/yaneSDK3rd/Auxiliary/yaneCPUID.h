//
//	CPU‚ÌŽ©“®”»•Ê
//

#ifndef __yaneCPUID_h__
#define __yaneCPUID_h__

class CCPUID {
public:
	int	GetID();

	CCPUID():m_nCPUID(0) {}
protected:
	int	m_nCPUID;
};

#endif
