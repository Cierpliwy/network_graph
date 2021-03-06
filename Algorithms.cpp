#include "Algorithms.h"
#include <queue>
#include <cstdlib>
#include <cmath>
using namespace std;

static Color getColor(float prob)
{
    Color c;
    c.r = (1-prob)*255;
    c.g = prob*255;
    c.b = 0;
    return c;
}

static float relVal(float min, float max, float n) {
    if (max == min) return n;
    return (n - min) / (max - min);
}

bool GraphAlgorithms::isConnected() {
    unsigned int unconnectedNum = 0;
    Node *n = m_graph->getFirstNode();
    while(n) {
        n->visited = false;
        if (!n->connected) unconnectedNum++;
        n = m_graph->getNextNode(n);
    }

    queue<Node*> visitedNodes;

    n = m_graph->getFirstNode();
    while(n && !n->connected) {
        n = m_graph->getNextNode(n);
    }
    if (!n) return false;

    n->visited = true;
    visitedNodes.push(n);
    unsigned int totalVisited = 1;

    while(!visitedNodes.empty()) {
        Node *n = visitedNodes.front();
        visitedNodes.pop();
        
        pair<Node*, Edge*> p = m_graph->getFirstAdjNode(n);
        while(p.first) {
            if (p.first->connected && p.second->connected && 
                !p.second->broken && !p.first->visited) {
                p.first->visited = true;
                visitedNodes.push(p.first);
                totalVisited++;
            }
            p = m_graph->getNextAdjNode(n);
        }
    }
    
    if (totalVisited == m_graph->getNodesNum() - unconnectedNum) return true;
    return false;

}

float GraphAlgorithms::monteCarlo()
{
    unsigned long long acc = 0;
    for (unsigned int i = 0; i < m_iterationsPerNode; ++i) {
        Edge *e = m_graph->getFirstEdge();
        while(e) {
            e->broken = ((rand() % 1001) / 1000.0f) > e->reliability; 
            e = m_graph->getNextEdge(e);
        }
        if (isConnected()) ++acc;
    }

    return acc/static_cast<float>(m_iterationsPerNode);
}

void GraphAlgorithms::edgeWeakness()
{
    Edge *lastEdge = nullptr;
    Edge *e = m_graph->getFirstEdge();
    Edge *er = nullptr;
    float minRel = 1, maxRel = 0;
    float minRelS = 1, maxRelS = 0;
    vector<pair<Edge*, float>> edges;

    while(e) {
       if (lastEdge) lastEdge->connected = true;
       e->connected = false;
       minRel = min(minRel, e->reliability);
       maxRel = max(maxRel, e->reliability);

       float rel = monteCarlo();
       if (rel < minRelS) er = e;
       minRelS = min(minRelS, rel);
       maxRelS = max(maxRelS, rel);

       edges.push_back(make_pair(e,rel));
       lastEdge = e;
       e = m_graph->getNextEdge(e);
    }
    if (lastEdge) lastEdge->connected = true;

    // Set style
    for(auto &e : edges) {
        e.first->style.penWidth = 
            3 * relVal(minRel, maxRel, e.first->reliability) + 1;

        if (m_relativeColors) {
            e.first->style.pen = getColor(relVal(minRelS, maxRelS, e.second));
        } else e.first->style.pen = getColor(e.second);

        if (e.first == er) e.first->style.dashed = true;
        else e.first->style.dashed = false;
    }
}

void GraphAlgorithms::nodeWeakness()
{
    Node *lastNode = nullptr;
    Node *n = m_graph->getFirstNode();
    float minRel = 1, maxRel = 0;
    Node *ns = nullptr;
    vector<pair<Node*, float>> nodes;

    while(n) {
        if (lastNode) lastNode->connected = true;
        n->connected = false;

        float rel = monteCarlo();
        if (rel < minRel) ns = n;
        minRel = min(minRel, rel);
        maxRel = max(maxRel, rel);

        nodes.push_back(make_pair(n,rel));
        lastNode = n;
        n = m_graph->getNextNode(n);
    }
    if (lastNode) lastNode->connected = true;

    //Set style
    for(auto &i : nodes) {
        if (i.first == ns) i.first->style.dashed = true;
        else i.first->style.dashed = false;

        i.first->style.penWidth = 
            3 * (1 - relVal(minRel, maxRel, i.second)) + 1;

        if (m_relativeColors)
            i.first->style.bg = getColor(relVal(minRel, maxRel, i.second));
        else
            i.first->style.bg = getColor(i.second);

    }
}
