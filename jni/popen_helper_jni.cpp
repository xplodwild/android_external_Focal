/**
 * Copyright (C) 2013 The CyanogenMod Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstdio>
#include <vector>
#include <string>

#include "mosaic/Log.h"
#define LOG_TAG "FocalProcess"

/**
 * JNI Declarations
 */
extern "C"
{
    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved);
    JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved);
    JNIEXPORT jint JNICALL Java_org_cyanogenmod_focal_PopenHelper_run(
            JNIEnv * env, jobject obj, jstring fileName);
};

/**
 * Helper functions
 */
void GetJStringContent(JNIEnv *AEnv, jstring AStr, std::string &ARes) {
	if (!AStr) {
		ARes.clear();
		return;
	}

	const char *s = AEnv->GetStringUTFChars(AStr,NULL);
	ARes=s;
	AEnv->ReleaseStringUTFChars(AStr,s);
}



/**
 * JNI Implementation
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    return JNI_VERSION_1_4;
}


JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved)
{

}

JNIEXPORT jint JNICALL Java_org_cyanogenmod_focal_PopenHelper_run(
            JNIEnv * env, jobject obj, jstring fileName)
{
	char line[256];
	
	// Convert JString to std::string
	std::string strFileName;
	GetJStringContent(env, fileName, strFileName);

	// Run the file specified
	FILE* fpipe = (FILE*) popen(strFileName.c_str(), "r");
	if (!fpipe)
	{
		LOGE("Cannot run %s! popen returned null", strFileName.c_str());
		return EXIT_FAILURE;
	}
	
	// Read the output
	while (fgets(line, sizeof line, fpipe))
	{
	   LOGI("%s", line);
	}
	
	pclose(fpipe);
	return EXIT_SUCCESS;
}



