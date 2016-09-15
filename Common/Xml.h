/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <Utils/ToString.h>
#include <Exception.h>
#include <map>
#include <vector>
#include <string>
#include <cstdio>
#include <stdint.h>

namespace XML
{
DECLARE_EXCEPTION(XmlException);

DECLARE_CHILD_EXCEPTION(NodeException, XmlException);
DECLARE_CHILD_EXCEPTION(NodeNotFoundException, NodeException);
DECLARE_CHILD_EXCEPTION(NodeExistException, NodeException);
DECLARE_CHILD_EXCEPTION(NodeIteratorException, NodeException);

DECLARE_CHILD_EXCEPTION(PropertyException, XmlException);
DECLARE_CHILD_EXCEPTION(PropertyNotFoundException, PropertyException);
DECLARE_CHILD_EXCEPTION(PropertyExistException, NodeException);

DECLARE_CHILD_EXCEPTION(XmlSyntaxException, XmlException);

class Node;
typedef std::vector<Node*> NodesSet;
typedef std::vector<const Node*> ConstNodesSet;
typedef std::map<std::string, NodesSet> NodesContainer;
typedef std::map<std::string, std::string> HeaderData;

struct NodesNamesData
{
    std::string name;
    size_t count;
    NodesNamesData(const std::string &Name, size_t Count) : name(Name), count(Count){}
    NodesNamesData() : count(0) {}
};

typedef std::vector<NodesNamesData> NodesNamesDataStorage;

template<class TNode>
class BaseNodeIterator
{
friend class Node;
private:
    TNode *owner;
    std::string curChildNodeName;
    int curChildNodeIndex;
    BaseNodeIterator(TNode *Owner, const std::string &ChildNodeName, int ChildNodeIndex)
        : owner(Owner), curChildNodeName(ChildNodeName), curChildNodeIndex(ChildNodeIndex)
    {}
    NodesContainer::const_iterator GetCiter() const
    {
        if(!owner)
            throw NodeIteratorException("Node iterator not initialised");

        if(owner->nodes.size() == 0)
            throw NodeIteratorException("Node " + owner->GetName() + " is empty");

        NodesContainer::const_iterator ci = owner->nodes.find(curChildNodeName);
        if(ci == owner->nodes.end())
            throw NodeIteratorException("Node " + curChildNodeName + " not found in " + owner->GetName() + " node");

        return ci;
    }
public:
    BaseNodeIterator() : owner(NULL), curChildNodeIndex(0) {}
    BaseNodeIterator<TNode>& operator++() throw (Exception)
    {
        NodesContainer::const_iterator ci = GetCiter();

        const NodesSet &nodes = ci->second;
        if(++curChildNodeIndex == nodes.size()){
            curChildNodeIndex = 0;

            NodesContainer::const_iterator tmpCi = ci;
            if(++tmpCi != owner->nodes.end())
                curChildNodeName = tmpCi->first;
            else
                curChildNodeName = "LastFor" + Utils::to_string(owner);
        }

        return *this;
    } 
    BaseNodeIterator<TNode> operator++(int Val) throw (Exception)
    {
        BaseNodeIterator<TNode> tmp = *this;
        operator++();
        return tmp;
    }
    TNode &operator*() const throw (Exception)
    {
        return *operator->();
    }
    TNode *operator->() const throw (Exception)
    {
        NodesContainer::const_iterator ci = GetCiter();

        const NodesSet &nodes = ci->second;
        if(!nodes.size())
            throw NodeIteratorException("Node " + curChildNodeName + " is empty");

        return nodes[curChildNodeIndex];
    }
    bool operator==(const BaseNodeIterator<TNode> &A)
    {
        return owner == A.owner && curChildNodeName == A.curChildNodeName && curChildNodeIndex == A.curChildNodeIndex;
    }
    bool operator!=(const BaseNodeIterator<TNode> &A)
    {
        return !operator==(A);
    }
};

typedef BaseNodeIterator<Node> NodeIterator;
typedef BaseNodeIterator<const Node> ConstNodeIterator;

class XmlData;
class NodeCopyCreater;
class NodeFileWriter;
class NodeStringAppender;
template<class TStorage>
class NodeFinder;

class Node final
{
friend class XmlData;
friend class NodeCopyCreater;
friend class NodeFileWriter;
friend class NodeStringAppender;
friend class NodeFinder<NodesSet>;
friend class NodeFinder<ConstNodesSet>;
friend class BaseNodeIterator<Node>;
friend class BaseNodeIterator<const Node>;
private:
    std::string name;
    std::string value;
    typedef std::map<std::string, std::string> PropertiesStorage;
    PropertiesStorage properties;
    NodesContainer nodes;
    void Construct(const Node &Var);
    Node * CreateCopy();
    void WriteToFile(FILE * File, int32_t &Level) const throw (XmlException);
    std::string ToString(int32_t &Level) const;
public:
    Node(){}
    ~Node();
    Node(const Node &Var);
    Node &operator= (const Node &Var);
    Node &GetNode(const std::string &NodeName, int32_t Index = 0) throw (XmlException);
    const Node &GetNode(const std::string &NodeName, int32_t Index = 0) const throw (XmlException);    
    std::string &GetProperty (const std::string &PropertyName) throw (XmlException);
    const std::string &GetProperty (const std::string &PropertyName) const throw (XmlException);    
    bool FindNode(const std::string &Name, ConstNodesSet &ConstNodes, bool Recursive = true) const;
    bool FindNode(const std::string &Name, NodesSet &Nodes, bool Recursive = true);
    bool FindNode(const std::string &PropName, const std::string &PropVal, ConstNodesSet &ConstNodes, bool Recursive = true) const;
    bool FindNode(const std::string &PropName, const std::string &PropVal, NodesSet &Nodes, bool Recursive = true);
    NodesNamesDataStorage GetNodesNames() const;
    std::vector<std::string> GetPropertiesNames() const;
    size_t GetNodesCount(const std::string &Name) const;
    void SetName(const std::string &Name) {name = Name;}
    const std::string &GetName() const { return name; }
    void SetValue(const std::string &Value) {value = Value;}
    const std::string &GetValue() const {return value;}
    void AddProperty(const std::string &Name, const std::string &Value) throw (XmlException);
    void RemoveProperty(const std::string &Name);
    void ClearProperties(){properties.clear();}
    void AddNode(const Node &NewNode);
    void RemoveNode(const std::string &Name, int32_t Ind = -1) throw (XmlException);
    void ClearNodes();
    NodeIterator begin();
    NodeIterator end();
    ConstNodeIterator begin() const;
    ConstNodeIterator end() const;
};

class XmlData final
{
private:  
    class Cursor
    {
    private:
        int coll, line;
    public:
        Cursor() : coll(1), line(1){}
        void ProcessChar(char Char);
        void Clear(){coll = line = 1;}
        XmlSyntaxException CreateException(const std::string &Message);
    };
    enum State
    {
        STATE_IDLE,        
        STATE_NODE_NAME,
        STATE_NODE_ATTR_NAME,
        STATE_NODE_ATTR_VAL
    };
    State state;
    bool stringState, shieldChar, isSingleNode, nodeClosed, varAsEmptyStr;
    enum HeaderState
    {
        HEADER_STATE_NOT_SET,
        HEADER_STATE_BEGIN, 
        HEADER_STATE_END
    };
    HeaderState headerState;
    enum StructState
    {
        STRUCT_STATE_NODE_DEFINITION,
        STRUCT_STATE_NODE_DATA
    };
    StructState structState;
    std::string text, varName, varData;
    std::vector<Node*> processingNodes;
    HeaderData header;
    Node *lastNode, rootNode;
    Cursor cursor;
    void ProcessChar(char Char) throw (XmlException);
    void ProcessNodeDefinitionStart() throw (XmlException);
    void ProcessNodeDefinitionEnd() throw (XmlException);
    void ProcessSlash() throw (XmlException);
    void ProcessSpace() throw (XmlException);
    void ProcessQuestionChar() throw (XmlException);
    void ProcessEqualChar() throw (XmlException);
    void Cleanup();
public:
    ~XmlData(){}
    XmlData();
    void LoadFromFile(const std::string &FilePath) throw (XmlException);
    void LoadFromString(const std::string &DataString) throw (XmlException);
    void SaveToFile(const std::string &FilePath) const throw (XmlException);
    std::string ToString() const;
    const Node &GetRoot() const { return rootNode;}
    Node &GetRoot() { return rootNode;}
    const HeaderData &GetHeaderData() const {return header;}
    void Clear() { rootNode.ClearNodes(); header.clear();}
};

};