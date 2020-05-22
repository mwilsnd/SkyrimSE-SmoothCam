#pragma once

namespace SkyrimSE {
	class Projectile : public TESObjectREFR {
		public:
			byte						dontCare0[0x10];		// 098
			float						unk0A8;					// 0A8
			float						unk0AC;					// 0AC
			float						unk0B0;
			float						unk0B4;
			float						unk0B8;					// 0B8
			float						unk0BC;					// 0BC
			float						unk0C0;
			float						unk0C4;
			float						unk0C8;					// 0C8
			float						unk0CC;					// 0CC
			float						unk0D0;
			float						unk0D4;
			float						unk0D8;					// 0D8
			float						unk0DC;					// 0DC
			bhkSimpleShapePhantom*		phantom;				// 0E0 - smart ptr
			mutable uintptr_t			unk0E8;					// 0E8
			NiPoint3					computedFlightNormal;	// 0F0
			NiPoint3					velocityVector;			// 0FC
			void*						unk108;					// 108 - smart ptr
			void*						unk110;					// 110 - smart ptr
			NiPointer<uint32_t>			actorCause;				// 118
			uint32_t					shooter;				// 120
			uint32_t					desiredTarget;			// 124
			byte						dontCare1[0xC];			// 128
			byte						dontCare2[0xC];			// 134
			UInt32*						unk140;					// 140
			InventoryEntryData*			unk148;					// 148
			UInt64						unk150;					// 150
			UInt64						unk158;					// 158
			UInt64						unk160;					// 160
			UInt64						unk168;					// 168
			NiPointer<QueuedFile>		projectileDBFiles;		// 170
			UInt64						unk178;					// 178
			UInt64						unk180;					// 180
			float						unk188;					// 188
			float						unk18C;					// 18C
			float						range;					// 190
			float						flightTimeAccumulator;	// 194
			float						unk198;					// 198
			float						unk19C;					// 19C
			UInt64						unk1A0;					// 1A0
			UInt64						unk1A8;					// 1A8
			TESObjectWEAP*				weaponSource;			// 1B0
			TESAmmo*					ammoSource;				// 1B8
			float						distanceMoved;			// 1C0
			UInt32						unk1C4;					// 1C4
			UInt32						unk1C8;					// 1C8
			UInt32						flags;					// 1CC
			UInt64						unk1D0;					// 1D0
	};
	static_assert(sizeof(Projectile) == 0x1D8);

	class MissileProjectile : public Projectile {
		public:
			uint32_t		impactResult;						// 1D8
			bool			waitingToInitialize3D;				// 1DC
			uint8_t			unk1DD;								// 1DD
			uint16_t		unk1DE;								// 1DE
	};
	static_assert(sizeof(MissileProjectile) == 0x1E0);
	
	class ArrowProjectile : public MissileProjectile {
		public:
			uint64_t unk1e0;
			AlchemyItem* poison;
	};
	static_assert(sizeof(ArrowProjectile) == 0x1F0);
}