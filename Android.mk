LOCAL_PATH := $(call my-dir)

include $(call all-makefiles-under,$(LOCAL_PATH))

include $(CLEAR_VARS)

$(shell mkdir -p $(OUT)/obj/SHARED_LIBRARIES/libmm-abl_intermediates/)
$(shell touch $(OUT)/obj/SHARED_LIBRARIES/libmm-abl_intermediates/export_includes)

# Create audio firmware links
$(shell mkdir -p $(TARGET_OUT_ETC)/firmware/wcd9320; \
    ln -sf /data/misc/audio/mbhc.bin  $(TARGET_OUT_ETC)/firmware/wcd9320/wcd9320_mbhc.bin; \
    ln -sf /data/misc/audio/wcd9320_anc.bin  $(TARGET_OUT_ETC)/firmware/wcd9320/wcd9320_anc.bin; \
    ln -sf /data/misc/audio/wcd9320_mad_audio.bin  $(TARGET_OUT_ETC)/firmware/wcd9320/wcd9320_mad_audio.bin)

# Create wifi firmware links
$(shell mkdir -p $(TARGET_OUT_ETC)/firmware/wlan/prima; \
    ln -sf /data/misc/wifi/WCNSS_qcom_cfg.ini  $(TARGET_OUT_ETC)/firmware/wlan/prima/WCNSS_qcom_cfg.ini; \
    ln -sf /data/misc/wifi/prima.bin  $(TARGET_OUT_ETC)/firmware/wlan/prima/prima.bin)

# Create wlan module links
$(shell mkdir -p $(TARGET_OUT)/lib/modules; \
    ln -sf /system/lib/modules/pronto/pronto_wlan.ko  $(TARGET_OUT)/lib/modules/wlan.ko)
