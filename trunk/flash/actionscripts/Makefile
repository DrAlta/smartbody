T07: check
	@echo "-------- Build Smartbody Data --------"

	$(AS3COMPILERARGS)	-import $(call nativepath,$(FLASCC)/usr/lib/builtin.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/playerglobal.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/BinaryData.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/ISpecialFile.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/IBackingStore.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/IVFS.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/AlcVFSZip.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/InMemoryBackingStore.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/PlayerKernel.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/CModule.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/C_Run.abc) \
				-import $(call nativepath,$(FLASCC)/usr/local/lib/libGL.abc) \
				Console.as -outdir . -out Console
				
		$(AS3COMPILERARGS)	-import $(call nativepath,$(FLASCC)/usr/lib/builtin.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/playerglobal.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/BinaryData.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/ISpecialFile.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/IBackingStore.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/IVFS.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/AlcVFSZip.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/CModule.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/C_Run.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/InMemoryBackingStore.abc) \
				-import $(call nativepath,$(FLASCC)/usr/lib/PlayerKernel.abc) \
				-import $(PWD)/Console.abc \
				Preloader.as -swf com.adobe.flascc.preloader.PreLoader,800,600,60 -outdir . -out PreLoader
include Makefile.common
