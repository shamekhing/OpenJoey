#include "stdafx.h"
#include "yaneSequencer.h"

bool	CSequenceInfo::operator <(const CSequenceInfo& x) const{
	if (x.m_nStart<m_nStart) return true;
	return false;
}


LRESULT CSequencer::Add(int start,int end,int data){
	CSequenceInfo	info;
	info.m_nStart	= start;
	info.m_nEnd		= end;
	info.m_nData	= data;
	m_oSequenceList.insert(info);
	return 0;
}


LRESULT CSequencer::Add(int start,int data){
	CSequenceInfo	info;
	info.m_nStart	= start;
	info.m_nEnd		= start + 0xffffff;	//	十分でかくとる！
	//	ソートは、m_nStartに従って行なわれるので、m_nEndの値は関係ない
	info.m_nData	= data;
	m_oSequenceList.insert(info);
	return 0;
}

LRESULT CSequencer::Del(int start,int end,int data){
	CSequenceInfo	info;
	info.m_nStart	= start;
	info.m_nEnd		= end;
	info.m_nData	= data;
	if (m_oSequenceList.erase(info) == 0 ) return 1; // not found..
	return 0;
}

void	CSequencer::Clear(void){
	m_oSequenceList.clear();
}

LRESULT	CSequencer::Get(int pos,int& data,int &diff){

	CSequenceInfo	info;
	info.m_nStart	= pos;
	set<CSequenceInfo>::iterator it;
	it = m_oSequenceList.lower_bound(info);
	if (it==m_oSequenceList.end()) return 1; // not found...

	// end boundary check...
	if ((it->m_nEnd)<=pos) return 1;		//	not founded...

	data = it->m_nData;
	diff = pos - it->m_nStart;
	return 0;
}
