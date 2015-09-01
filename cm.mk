# Boot animation
TARGET_SCREEN_HEIGHT := 1920
TARGET_SCREEN_WIDTH := 1080

# Inherit some common CM stuff.
$(call inherit-product, vendor/cm/config/common_full_phone.mk)

# Enhanced NFC
$(call inherit-product, vendor/cm/config/nfc_enhanced.mk)

# Inherit device configuration
$(call inherit-product, device/pantech/a890/full_a890.mk)

## Device identifier. This must come after all inclusions
PRODUCT_DEVICE := a890
PRODUCT_NAME := cm_a890
PRODUCT_BRAND := VEGA
PRODUCT_MODEL := IM-A890
PRODUCT_MANUFACTURER := PANTECH

PRODUCT_DEFAULT_LANGUAGE := zh
PRODUCT_DEFAULT_REGION := CN
#PRODUCT_LOCALES := zh_CN zh_TW en_US

PRODUCT_BUILD_PROP_OVERRIDES += BUILD_FINGERPRINT=VEGA/VEGA_IM-A890L/EF59L:4.4.2/KVT49L/IM-A890L.012:user/release-keys PRIVATE_BUILD_DESC="msm8974-user 4.4.2 KVT49L IM-A890L.012 release-keys"

# Enable Torch
PRODUCT_PACKAGES += Torch
