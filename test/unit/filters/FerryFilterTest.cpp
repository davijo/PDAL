/******************************************************************************
* Copyright (c) 2014, Howard Butler (howard@hobu.co)
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/

#include <pdal/pdal_test_main.hpp>

#include <pdal/PointView.hpp>
#include <pdal/StageFactory.hpp>
#include <pdal/PipelineManager.hpp>
#include <pdal/PipelineReaderJSON.hpp>
#include <pdal/PipelineReaderXML.hpp>
#include <FerryFilter.hpp>
#include <LasReader.hpp>
#include "Support.hpp"

using namespace pdal;

TEST(FerryFilterTest, create)
{
    StageFactory f;
    std::unique_ptr<Stage> filter(f.createStage("filters.ferry"));
    EXPECT_TRUE(filter.get());
}

TEST(FerryFilterTest, test_ferry_copy_xml)
{
    PipelineManager mgr;
    PipelineReaderXML specReader(mgr);
    specReader.readPipeline(Support::configuredpath("filters/ferry.xml"));

    mgr.execute();
    ConstPointTableRef table(mgr.pointTable());

    PointViewSet viewSet = mgr.views();

    EXPECT_EQ(viewSet.size(), 1u);
    PointViewPtr view = *viewSet.begin();
    EXPECT_EQ(view->size(), 1065u);

    Dimension::Id::Enum state_plane_x = table.layout()->findDim("StatePlaneX");
    Dimension::Id::Enum state_plane_y = table.layout()->findDim("StatePlaneY");

    double lon = view->getFieldAs<double>(Dimension::Id::X, 0);
    double lat = view->getFieldAs<double>(Dimension::Id::Y, 0);

    double x = view->getFieldAs<double>(state_plane_x, 0);
    double y = view->getFieldAs<double>(state_plane_y, 0);

    EXPECT_DOUBLE_EQ(-117.2501328350574, lon);
    EXPECT_DOUBLE_EQ(49.341077824192915, lat);
    EXPECT_DOUBLE_EQ(637012.24, x);
    EXPECT_DOUBLE_EQ(849028.31, y);
}

TEST(FerryFilterTest, test_ferry_copy_json)
{
    PipelineManager mgr;
    PipelineReaderJSON specReader(mgr);
    specReader.readPipeline(Support::configuredpath("filters/ferry.json"));

    mgr.execute();
    ConstPointTableRef table(mgr.pointTable());

    PointViewSet viewSet = mgr.views();

    EXPECT_EQ(viewSet.size(), 1u);
    PointViewPtr view = *viewSet.begin();
    EXPECT_EQ(view->size(), 1065u);

    Dimension::Id::Enum state_plane_x = table.layout()->findDim("StatePlaneX");
    Dimension::Id::Enum state_plane_y = table.layout()->findDim("StatePlaneY");

    double lon = view->getFieldAs<double>(Dimension::Id::X, 0);
    double lat = view->getFieldAs<double>(Dimension::Id::Y, 0);

    double x = view->getFieldAs<double>(state_plane_x, 0);
    double y = view->getFieldAs<double>(state_plane_y, 0);

    EXPECT_DOUBLE_EQ(-117.2501328350574, lon);
    EXPECT_DOUBLE_EQ(49.341077824192915, lat);
    EXPECT_DOUBLE_EQ(637012.24, x);
    EXPECT_DOUBLE_EQ(849028.31, y);
}

TEST(FerryFilterTest, test_ferry_invalid)
{
    Options ops1;
    ops1.add("filename", Support::datapath("las/1.2-with-color.las"));
    LasReader reader;
    reader.setOptions(ops1);

    Options op1;

    op1.add("dimensions", "X=X");

    FerryFilter f1;
    f1.setInput(reader);
    f1.setOptions(op1);

    PointTable table;

    // Make sure we can't ferry to ourselves.
    EXPECT_THROW(f1.prepare(table), pdal_error);

    Options op2;

    op2.add("dimension", "X=NewX");
    FerryFilter f2;
    f2.setInput(reader);
    f2.setOptions(op2);

    // Make sure we reject old option name.
    EXPECT_THROW(f2.prepare(table), pdal_error);

    Options op3;

    op3.add("dimensions", "NewX = X");
    FerryFilter f3;
    f3.setInput(reader);
    f3.setOptions(op3);

    // Make sure we reject bad source dimension.
    EXPECT_THROW(f3.prepare(table), pdal_error);

    Options op4;

    op4.add("dimensions", "X = Y, X = NewZ = NewQ");
    FerryFilter f4;
    f4.setInput(reader);
    f4.setOptions(op4);

    // Make sure we reject bad option format.
    EXPECT_THROW(f4.prepare(table), pdal_error);
}
