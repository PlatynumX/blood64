#pragma once

#define __N64__ 1
#define BLOOD64 1
#define ENGINE_19960925 1
#define NETCODE_DISABLE 1
#define SMACKER_DISABLE 1
#define NOASM 1

/* The first N64 gameplay target is the classic renderer only. */
#ifndef USE_OPENGL
#define USE_OPENGL 0
#endif
#ifndef POLYMER
#define POLYMER 0
#endif

/* Deliberately constrained retail/shareware target. */
#define BLOOD64_XDIM 320
#define BLOOD64_YDIM 240
#define BLOOD64_REQUIRE_EXPANSION_PAK 1
