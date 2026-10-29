#!/bin/bash
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Generate a signed .p7b profile matching the bundleName in AppScope/app.json5,
# using the generic OpenHarmony test materials in developtools/hapsigner/dist
# (the same ones the GN build signs with, see build/ohos_var.gni).
#
# The p7b file embeds the bundle-name and must match the hap's bundleName,
# otherwise installation fails with "verify signature failed (code:9568329)".
#
# Usage:   bash generate_signature.sh
# Needs:   Java 11+ (java -version), developtools/hapsigner repo synced.
# Adapted from hiviewdfx_hiview PR !4552 (faultlogger_hap/generate_signature.sh).

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../../../../../" && pwd)"
SIGN_DIR="$REPO_ROOT/developtools/hapsigner/dist"

for f in hap-sign-tool.jar OpenHarmony.p12 OpenHarmonyProfileRelease.pem UnsgnedReleasedProfileTemplate.json; do
    if [ ! -f "$SIGN_DIR/$f" ]; then
        echo "ERROR: $f not found at $SIGN_DIR"
        echo "Ensure developtools/hapsigner repo is synced."
        exit 1
    fi
done

APP_JSON5="$SCRIPT_DIR/AppScope/app.json5"
if [ ! -f "$APP_JSON5" ]; then
    echo "ERROR: $APP_JSON5 not found"
    exit 1
fi

BUNDLE_NAME=$(python3 -c "
import json, re
with open('$APP_JSON5') as f:
    content = f.read()
content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
content = re.sub(r'//.*?\n', '\n', content)
content = content.rstrip().rstrip(',')
data = json.loads(content)
print(data['app']['bundleName'])
")

echo "Bundle name: $BUNDLE_NAME"

TMP_PROFILE="$SCRIPT_DIR/signature/profile_template.json"
mkdir -p "$SCRIPT_DIR/signature"
python3 -c "
import json
with open('$SIGN_DIR/UnsgnedReleasedProfileTemplate.json') as f:
    data = json.load(f)
data['bundle-info']['bundle-name'] = '$BUNDLE_NAME'
with open('$TMP_PROFILE', 'w') as f:
    json.dump(data, f, indent=4)
print('Profile JSON created: $TMP_PROFILE')
"

P7B_OUT="$SCRIPT_DIR/signature/openharmony_sx.p7b"
echo "Generating p7b..."
java -jar "$SIGN_DIR/hap-sign-tool.jar" sign-profile \
    -mode localSign \
    -keyAlias "openharmony application profile release" \
    -signAlg SHA256withECDSA \
    -profileCertFile "$SIGN_DIR/OpenHarmonyProfileRelease.pem" \
    -inFile "$TMP_PROFILE" \
    -keystoreFile "$SIGN_DIR/OpenHarmony.p12" \
    -outFile "$P7B_OUT" \
    -keyPwd 123456 \
    -keystorePwd 123456

echo "Generated: $P7B_OUT"

echo "Verifying bundle-name in p7b..."
if strings "$P7B_OUT" | grep -q "\"bundle-name\":\"$BUNDLE_NAME\""; then
    echo "OK: bundle-name matches ($BUNDLE_NAME)"
else
    echo "WARNING: bundle-name verification failed, check the p7b file"
fi

rm -f "$TMP_PROFILE"
echo "Done."
