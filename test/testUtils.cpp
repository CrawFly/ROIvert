#include "testUtils.h"
#include "VideoData.h"
#include <QDebug>

void loaddataset(VideoData* data, datasettype type, double framerate, int downspace, int downtime)
{
    if (!data) {
        return;
    }

    std::vector<std::pair<QString, size_t>> filenameframelist;
    QString fp = TEST_RESOURCE_DIR;
    QString prefix = "roivert_testdata_";
    QString base = fp + "/" + prefix;

    switch (type)
    {
    case datasettype::ONESTACK:
        filenameframelist.push_back({ base + "onestack.tiff", 8 });
        break;
    case datasettype::MULTIPLESTACKS:
        filenameframelist.push_back({ base + "multistack_1.tiff", 2 });
        filenameframelist.push_back({ base + "multistack_2.tiff", 2 });
        filenameframelist.push_back({ base + "multistack_3.tiff", 2 });
        filenameframelist.push_back({ base + "multistack_4.tiff", 2 });
        break;
    case datasettype::SINGLEFRAMES:
        for (size_t i = 0; i < 8; ++i) {
            filenameframelist.push_back({ base + "singleframe_" + QString::number(i+1) + ".tiff", 1 });
        }
        break;
    case datasettype::DEADPIX:
        filenameframelist.push_back({ base + "onestack_deadpix.tiff", 8 });
        break;
    }
    // todo: ASSERT FILES EXIST

    data->load(filenameframelist, downtime, downspace);
    data->setFrameRate(framerate);
}