#ifndef KF_PSYQ_CDSTREAM_H
#define KF_PSYQ_CDSTREAM_H

/*
 * LIBCD streaming state that Runtime Library 4.2's LIBCD.H does not declare.
 * Sony's movie sample declares StCdIntrFlag itself before calling
 * StCdInterrupt from its DecDCTout callback; the flag is a LIBCD COMMON.
 */
extern int StCdIntrFlag;

#endif
