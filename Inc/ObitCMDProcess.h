
typedef struct
{
	char *Obit_CMD;
	uint8_t  Obit_CMD_Len;
	uint8_t  Value_CMD;

} SetUp;


#define DataLog_Num 5

SetUp DataLoggerSetting[DataLog_Num] =
{
	{NULL,0,0},
	{"domain",6,1},
	{"time",4,2},
	{"token",5,3},
	{"phone",5,4}
};

void ObitCMD_Setting(uint8_t *SMS_Content, uint8_t SMS_Content_Len)
{
	uint8_t CMD_name[6], CMD_value[100];
	uint8_t real_len_CMD, value_setting;
  obit_split_string(SMS_Content,SMS_Content_Len,'\n',2,':',1,CMD_name,sizeof(CMD_name));
	obit_split_string(SMS_Content,SMS_Content_Len,':',4,'\r',1,CMD_value, sizeof(CMD_value));
	real_len_CMD = strlen(CMD_name);
	for(int i=0; i<DataLog_Num; i++)
	{
		if(real_len_CMD == DataLoggerSetting[i].Obit_CMD_Len)
		{
			if(obit_strcmp(DataLoggerSetting[i].Obit_CMD,DataLoggerSetting[i].Obit_CMD_Len,CMD_name,sizeof(CMD_name)))
			{
				value_setting = DataLoggerSetting[i].Value_CMD;
			}
		}
	}
  switch (value_setting)
	 {
		 case 1: // set domain
		 {
			 
			 break;
		 }
		 case 2: // set time response
		 {
			 break;
		 }
		 case 3: // set token
		 {
			 break;
		 }
		 case 4: // set phone server
		 {
			 break;
		 }
	 }
	 
}