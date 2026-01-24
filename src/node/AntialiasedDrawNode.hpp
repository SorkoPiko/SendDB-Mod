#ifndef SENDDB_ANTIALIASEDDRAWNODE_HPP
#define SENDDB_ANTIALIASEDDRAWNODE_HPP

#include <Geode/cocos/draw_nodes/CCDrawNode.h>

class AntialiasedDrawNode : public CCDrawNode {
public:
    CREATE_FUNC(AntialiasedDrawNode);

#ifndef GEODE_IS_MOBILE
    void draw() override {
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_POLYGON_SMOOTH);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_MULTISAMPLE);

        CCDrawNode::draw();

        glDisable(GL_POLYGON_SMOOTH);
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_MULTISAMPLE);
    }
#endif
};

#endif