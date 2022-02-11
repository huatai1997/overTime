#include "getinfo.h"
#include "file.h"
#include "json.h"

#define DATE_STR       "date"
#define RESULT_STR     "result"
#define SIGNINFO_STR   "signInfo"
#define SIGNTIME_STR   "signTime"
#define WORKFLOW_STR   "workflow"
#define COUNT_STR      "count"
#define TITLE_STR      "title"
#define ISWORKDAY_STR  "isWorkDay"

/**
 * 将json数据解析到结构体数组指针中
 * @param root [root] json数据指针
 * @param map [in] 结构体数组指针
 */
static void parsing_json(json_t *root, mouthInfo *map)
{
	json_t *result = json_find_first_label(root, RESULT_STR);
    if (result != NULL)
    {
        int num;
		int hour, min, sec;
        char numBuff[4] = {0};
        json_t *tmp = NULL;

        for (num = 1; num < 32; num++)
        {
            memset(numBuff, 0, 4);

            snprintf(numBuff, 4, "%d", num);
            tmp = json_find_first_label(result->child, numBuff);
            if (tmp != NULL)
            {
                json_t *signInfo = json_find_first_label(tmp->child, SIGNINFO_STR);
                json_t *isWorkDay = json_find_first_label(tmp->child, ISWORKDAY_STR);
				json_t *date = json_find_first_label(tmp->child, DATE_STR);
				json_t *workflow = json_find_first_label(tmp->child, WORKFLOW_STR);

				strcpy(map->dayInfoList[num-1].date, date->child->text);
                map->dayInfoList[num-1].isWork = (isWorkDay->child->type == JSON_TRUE);
				map->dayInfoList[num-1].isSign = (signInfo != NULL);

				if ( workflow != NULL &&
					 workflow->child != NULL && 
					 workflow->child->child != NULL
				   )
				{
					int workflowValue = 0;
					json_t *workflowItem = workflow->child->child;
					while (workflowItem != NULL)
					{
						json_t *count = json_find_first_label(workflowItem, COUNT_STR);
						double hours = atof(count->child->text);

						workflowValue += (int)(hours * 3600.00);

						workflowItem = workflowItem->next;
					}

					map->dayInfoList[num-1].workflow = workflowValue;
				}
				else
				{
					map->dayInfoList[num-1].workflow = -1;
				}

                if (signInfo != NULL)
                {
                    json_t *signInfoItem = signInfo->child->child;
                    while (signInfoItem != NULL)
                    {
                        json_t *signTime = json_find_first_label(signInfoItem, SIGNTIME_STR);
                        json_t *title = json_find_first_label(signInfoItem, TITLE_STR);

						if (0 == strcmp(title->child->text, "上班打卡"))
                        {
							if (0 == strcmp(signTime->child->text, "未打卡"))
							{
								map->dayInfoList[num-1].timeToWork = -1;
							}
							else
							{
								sscanf(signTime->child->text, "%d:%d:%d", &hour, &min, &sec);
								map->dayInfoList[num-1].timeToWork = hour * 3600 + min * 60 + sec;
							}
                        }
                        else
                        {
							if (0 == strcmp(signTime->child->text, "未打卡"))
							{
								map->dayInfoList[num-1].timeFromWork = -1;
							}
							else
							{
								sscanf(signTime->child->text, "%d:%d:%d", &hour, &min, &sec);
								if (hour >= 0 && hour <= 8)
								{
									hour += 24;
								}

								map->dayInfoList[num-1].timeFromWork = hour * 3600 + min * 60 + sec;
							}
                        }

						signInfoItem = signInfoItem->next;
                    }
                }
            }
        }
    }
}

/**
 * 将文件数据保存到指定空间
 * @param file [in] 文件路径
 * @param map [in] 结构体指针
 * @return 整数 -1=失败 其他表示成功
 */
int save_work_info_to_map(const char *file, mouthInfo *map)
{
	if (NULL == map || NULL == file)
	{
		return -1;
	}

	char *fileInfo = read_info_from_file(file);
	if (NULL == fileInfo)
	{
		return -1;
	}

	json_t *fileInfoJson = NULL;
	json_parse_document(&fileInfoJson, fileInfo);
	free(fileInfo);
	if (NULL == fileInfoJson)
	{
		return -1;
	}

	parsing_json(fileInfoJson, map);

	json_free_value(&fileInfoJson);

	return 0;
}
