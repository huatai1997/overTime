#include "getinfo.h"
#include "calcovertime.h"

void usage(void)
{
	printf("\n\tusage : getOverTime -f json.txt\n\n");
	exit(-1);
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		usage();
	}

	mouthInfo mt;
	memset(&mt, 0, sizeof(mouthInfo));

	save_work_info_to_map(argv[2], &mt);

	int overTimeOfMonth = get_over_time_from_map(&mt);

	printf("\nHi:\n\tYou had worked \033[1;40;33m%d\033[0m hours \033[1;40;33m%d\033[0m minutes \033[1;40;33m%d\033[0m seconds of overtime this month!\n\n",
                 overTimeOfMonth / 3600, (overTimeOfMonth % 3600) / 60, overTimeOfMonth % 60);

	return 0;
}
