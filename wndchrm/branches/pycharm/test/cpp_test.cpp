
#include "mfg_draw_graph.h"
#include "mfg_lexer.h"
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    const char* p_srcdir = getenv("srcdir");
    std::string srcdir;
    if (!p_srcdir)
    {
        srcdir = ".";
    }
    else
    {
        srcdir = std::string(p_srcdir) + "/test";
    }
    std::string xdot_name = srcdir + "/test.xdot";
    mfg::DrawGraph draw_graph = mfg::DrawGraph();
    std::ifstream f(xdot_name.c_str());
    if (!f.is_open() || !draw_graph.LoadFromXdot(f))
    {
        std::cerr << "Could not load XDOT file '" << xdot_name << "'" << std::endl;
        return 1;
    }

    const char* expected_labels[] = {
        "mfg::Graph",
        "mfg::DrawGraph",
        "TmfGraph",
        " { flex | bison }",
        "TScrollBox",
        "DOT.EXE"
    };

    const mfg::NodeList& node_list = draw_graph.GetNodeList();
    mfg::NodeList::const_iterator node_it;
    int expected_index = 0;
    for (node_it = node_list.begin();
         node_it != node_list.end();
         ++node_it, ++expected_index)
    {
        std::string received_label = (*node_it)->Attribs()["label"];
        std::string expected_label = expected_labels[expected_index];
        if (received_label != expected_label)
        {
            std::cerr << "expected '" << expected_label
                << "', got '" << received_label << "'" << std::endl;
            return 2;
        }
    }
    return 0;
}
