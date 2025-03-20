//	yaneSinTable.h :
//
//		Sin Table
//

#ifndef __yaneSinTable_h__
#define __yaneSinTable_h__

class CSinTable {
/**
	sin,cosのテーブルを提供します。高速なsin,cosが実現できます。
	0～511で一周（360゜）	結果は<<16されて返ります。
*/
public:
	///	n:0-511で一周。結果は<<16されて返る
	LONG	Cos(int n){ return m_lTable[n & 511]; }
	LONG	Sin(int n){ return m_lTable[(n+384) & 511]; }

	CSinTable();

protected:
	LONG	m_lTable[512];	// cos table
};

#endif
