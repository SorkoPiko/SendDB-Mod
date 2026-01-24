#ifndef SENDDB_ANTIALIASEDDRAWNODE_HPP
#define SENDDB_ANTIALIASEDDRAWNODE_HPP

#include <Geode/cocos/draw_nodes/CCDrawNode.h>

class AntialiasedDrawNode : public CCDrawNode {
public:
    CREATE_FUNC(AntialiasedDrawNode);

    void draw() override {
#ifndef GEODE_IS_MOBILE
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_POLYGON_SMOOTH);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
#else
        glEnable(GL_MULTISAMPLE);
#endif

        CCDrawNode::draw();

#ifndef GEODE_IS_MOBILE
        glDisable(GL_POLYGON_SMOOTH);
        glDisable(GL_LINE_SMOOTH);
#else
        glDisable(GL_MULTISAMPLE);
#endif
    }
};

#endif