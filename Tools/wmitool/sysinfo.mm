//
//  sysinfo.mm
//  wmitool
//
//  Ported from libsmbios by lvs1974 on 18/10/2022.
//  Copyright © 2022 lvs1974. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <sstream>
#import <vector>
#import <string>
#import <ctype.h>
#import "wmi.h"

#define MAX_SMI_TAG_SIZE 12

void strip_trailing_whitespace( char *str )
{
	if(!str)
		return;

	if(strlen(str) == 0)
		return;

	size_t ch = strlen(str);
	do
	{
		--ch;
		if( ' ' == str[ch] )
			str[ch] = '\0';
		else
			break;

	} while(ch);
}

int get_service_asset_tag(uint32_t selector, char *service_asset_tag)
{
	calling_interface_buffer in  = {};
	calling_interface_buffer out = {};
	size_t outsize = sizeof(out);
	in.cmd_class  = 11;
	in.cmd_select = selector;

	kern_return_t ret = IOConnectCallStructMethod(connect, actionEvaluate, &in, sizeof(in), &out, &outsize);
	if (ret != KERN_SUCCESS) {
		printf("Can't connect to StructMethod to send commands\n");
		return 1;
	}
	else {
		if (out.output[0] != 0) {
			return 1;
		} else {
			memcpy(service_asset_tag, (uint8_t *)(&(out.output[1])), MAX_SMI_TAG_SIZE);
			
			for(size_t i=0; i<MAX_SMI_TAG_SIZE; ++i)
				if ((unsigned char)(service_asset_tag[i])==(unsigned char)0xFF)
					service_asset_tag[i] = '\0';
			
			strip_trailing_whitespace(service_asset_tag);
		}
	}
	
	return 0;
}

int PrintSysInfo()
{
	char service_tag[MAX_SMI_TAG_SIZE + 1] = {};
	char asset_tag[MAX_SMI_TAG_SIZE + 1] = {};
	if (get_service_asset_tag(2, service_tag) == 0)
		printf("Service Tag:  %s\n", service_tag);
	else
		printf("Service Tag:  Not Specified\n");

	if (get_service_asset_tag(0, asset_tag) == 0)
		printf("Asset Tag:  %s\n", asset_tag);
	else
		printf("Asset Tag:  Not Specified\n");

	return 0;
}
