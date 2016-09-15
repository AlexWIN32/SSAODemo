/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <Xml.h>
#include <Utils/FileGuard.h>
#include <Utils/AutoEvent.h>
#include <ctype.h>
#include <sstream>

namespace XML
{

static void write_to_file(const std::string &String, FILE *File, int32_t Level) throw (Exception)
{
    std::string str = std::string(Level * 4, ' ' ) + String + "\n";

    if(fwrite(str.c_str(), 1, str.length(), File) != str.length())
        throw XmlException("cant write to file");
}

static std::string to_string(const std::string &String, int32_t Level)
{
    return std::string(Level * 4, ' ') + String + "\n";
}

static std::string get_propertis_string(const std::map<std::string, std::string> &Properties)
{
    std::string propsStr;

    std::map<std::string, std::string>::const_iterator ci;
    for(ci = Properties.begin(); ci != Properties.end(); ++ci){
        if(ci != Properties.begin())
            propsStr += " ";
        propsStr += ci->first + "=\"" + ci->second + "\"";
    }    
    return (propsStr != "") ? " " + propsStr : propsStr;
}

static std::string header_to_string(const std::map<std::string, std::string> &HeaderData)
{
    return "<?xml" + get_propertis_string(HeaderData) + "?>";
}

template<class Iterator, class Function, class ContainerType = NodesSet>
void for_each_node(Iterator First, Iterator Last, Function Func )
{
    while(First != Last){
        const std::string nodesName = First->first;
        const ContainerType &set = First->second;

        typename ContainerType::const_iterator sci;
        for(sci = set.begin(); sci != set.end(); ++sci)
            Func(*sci);
        ++First;
    }
}

class NodeCopyCreater
{
private:
    NodesContainer * nodes;
public:
    NodeCopyCreater(NodesContainer * Nodes): nodes(Nodes) {}
    void operator() (Node * ProcessNode)
    {
        (*nodes)[ProcessNode->GetName()].push_back(ProcessNode->CreateCopy());
    }
};

class NodeFileWriter
{
private:
    FILE * file;
    int32_t level;
public:
    NodeFileWriter(FILE * File, int32_t Level): file(File), level(Level) {}
    void operator() (Node * ProcessNode)
    {
        ProcessNode->WriteToFile(file, level);
    }
    void operator() (const Node &ProcessNode)
    {
        ProcessNode.WriteToFile(file, level);
    }
};

class NodeStringAppender
{
private:
    std::string * str;
    int32_t level;
public:
    NodeStringAppender(std::string * Str, int32_t Level): str(Str), level(Level){}
    void operator() (Node * ProcessNode)
    {
        *str += ProcessNode->ToString(level);
    }    
};

template<class TStorage>
class NodeFinder
{
private:
    TStorage *storage;
    const std::string *name;
    const std::string *propVar, *propVal;
    bool recursive;
public:
    NodeFinder(const std::string *Name, TStorage *Storage, bool Recursive) 
        : name(Name), storage(Storage), recursive(Recursive), propVar(NULL), propVal(NULL)
    {}
    NodeFinder(const std::string *PropVar, const std::string *PropVal, TStorage *Storage, bool Recursive)
        : name(NULL), storage(Storage), recursive(Recursive), propVar(PropVar), propVal(PropVal)
    {}
    void operator() (Node * ProcessNode)
    {        
        if(name){
            if(ProcessNode->name == *name)
                storage->push_back(ProcessNode);        
        }else{
            const Node::PropertiesStorage &properties = ProcessNode->properties;
            Node::PropertiesStorage::const_iterator ci;
            for(ci = properties.begin(); ci != properties.end(); ++ci)
                if(ci->first == *propVar && ci->second == *propVal){
                    storage->push_back(ProcessNode);
                    break;
                }            
        }

        if(recursive){
            const NodesContainer &nodes = ProcessNode->nodes;
            for_each_node(nodes.begin(), nodes.end(), NodeFinder(*this));
        }
    }
};

Node * Node::CreateCopy()
{
    Node * newNode = new Node();
    newNode->name = name;
    newNode->properties = properties;

    for_each_node(nodes.begin(), nodes.end(), NodeCopyCreater(&newNode->nodes));

    return newNode;
}

void Node::Construct(const Node &Var)
{
    name = Var.name;
    properties = Var.properties;

    for_each_node(Var.nodes.begin(), Var.nodes.end(), NodeCopyCreater(&nodes));
}

Node::~Node()
{
    for_each_node(nodes.begin(), nodes.end(), [&](Node * ProcessNode) { delete ProcessNode;} );
}

Node::Node(const Node &Var)
{
    Construct(Var);
}

Node &Node::operator= (const Node &Var)
{
    if(this != &Var)
        Construct(Var);

    return *this;
}

Node &Node::GetNode(const std::string &NodeName, int32_t Index) throw (XmlException)
{
    NodesContainer::iterator it = nodes.find(NodeName);
    if(it == nodes.end())
        throw NodeNotFoundException(std::string("Node ") + NodeName + " not found");

    NodesSet &set = it->second;

    if(Index < 0 || Index >= set.size())
        throw NodeNotFoundException("Invalid index for node " + NodeName);

    return *set[Index];
}

const Node &Node::GetNode(const std::string &NodeName, int32_t Index) const throw (XmlException)
{
    NodesContainer::const_iterator ci = nodes.find(NodeName);
    if(ci == nodes.end())
        throw NodeNotFoundException(std::string("Node ") + NodeName + " not found");

    const NodesSet &set = ci->second;

    if(Index < 0 || Index >= set.size())
        throw NodeNotFoundException("Invalid index for node " + NodeName);

    return *set[Index];
}

std::string &Node::GetProperty(const std::string &PropertyName) throw (XmlException)
{
    PropertiesStorage::iterator it = properties.find(PropertyName);
    if(it == properties.end())
        throw PropertyNotFoundException(std::string("Property ") + PropertyName + " not found");

    return it->second;
}

const std::string &Node::GetProperty(const std::string &PropertyName) const throw (XmlException)
{
    PropertiesStorage::const_iterator ci = properties.find(PropertyName);
    if(ci == properties.end())
        throw PropertyNotFoundException(std::string("Property ") + PropertyName + " not found");

    return ci->second;
}

bool Node::FindNode(const std::string &Name, ConstNodesSet &ConstNodes, bool Recursive) const
{        
    ConstNodesSet findedNodes;
    NodeFinder<ConstNodesSet> finder(&Name, &findedNodes, Recursive);
    for_each_node(nodes.begin(), nodes.end(), finder);

    ConstNodes.insert(ConstNodes.end(), findedNodes.begin(), findedNodes.end());
    return findedNodes.size() != 0;    
}

bool Node::FindNode(const std::string &Name, NodesSet &Nodes, bool Recursive)
{    
    NodesSet findedNodes;
    NodeFinder<NodesSet> finder(&Name, &findedNodes, Recursive);
    for_each_node(nodes.begin(), nodes.end(), finder);

    Nodes.insert(Nodes.end(), findedNodes.begin(), findedNodes.end());
    return findedNodes.size() != 0;
}

bool Node::FindNode(const std::string &PropName, const std::string &PropVal, ConstNodesSet &ConstNodes, bool Recursive) const
{
    ConstNodesSet findedNodes;
    NodeFinder<ConstNodesSet> finder(&PropName, &PropVal, &findedNodes, Recursive);
    for_each_node(nodes.begin(), nodes.end(), finder);

    ConstNodes.insert(ConstNodes.end(), findedNodes.begin(), findedNodes.end());
    return findedNodes.size() != 0;   
}

bool Node::FindNode(const std::string &PropName, const std::string &PropVal, NodesSet &Nodes, bool Recursive)
{
    NodesSet findedNodes;
    NodeFinder<NodesSet> finder(&PropName, &PropVal, &findedNodes, Recursive);
    for_each_node(nodes.begin(), nodes.end(), finder);

    Nodes.insert(Nodes.end(), findedNodes.begin(), findedNodes.end());
    return findedNodes.size() != 0;
}

NodesNamesDataStorage Node::GetNodesNames() const
{
    NodesNamesDataStorage nodeNames;

    NodesContainer::const_iterator ci;
    for(ci = nodes.begin(); ci != nodes.end(); ++ci)
        nodeNames.push_back(NodesNamesData(ci->first, ci->second.size()));

    return nodeNames;
}

size_t Node::GetNodesCount(const std::string &Name) const
{
    NodesContainer::const_iterator ci = nodes.find(Name);
    if(ci == nodes.end())
        return 0;

    return ci->second.size();
}

std::vector<std::string> Node::GetPropertiesNames() const
{
    std::vector<std::string> propsNames;

    PropertiesStorage::const_iterator ci;
    for(ci = properties.begin(); ci != properties.end(); ++ci)
        propsNames.push_back(ci->first);

    return propsNames;
}

void Node::AddProperty(const std::string &Name, const std::string &Value) throw (XmlException)
{
    PropertiesStorage::const_iterator ci = properties.find(Name);
    if(ci != properties.end())
        throw PropertyExistException(std::string("Property ") + Name + " already exist in node " + name);

    properties.insert(std::make_pair(Name, Value));   
}

void Node::RemoveProperty(const std::string &Name)
{
    properties.erase(Name);
}

void Node::AddNode(const Node &NewNode)
{    
    nodes[NewNode.GetName()].push_back(new Node(NewNode));
}

void Node::RemoveNode(const std::string &Name, int32_t Ind) throw (XmlException)
{
    NodesContainer::iterator it = nodes.find(Name);
    if(it == nodes.end())
        return;

    NodesSet &set = it->second;
    NodesSet::iterator sit = set.begin();

    if(Ind == -1){
        
        for(; sit != set.end(); delete *sit++);

        nodes.erase(it);
    }else{
        if(Ind < 0 || Ind > set.size())
            throw XmlException("Invalid deleting index");

        std::advance(sit, Ind);
        delete *sit;
        
        set.erase(sit);
        if(!set.size())
            nodes.erase(it);
    }
    
}

void Node::WriteToFile(FILE * File, int32_t &Level) const throw (XmlException)
{
    if(!nodes.size() && value == "")
        write_to_file("<" + name + get_propertis_string(properties) + "/>", File, Level);
    else{
        write_to_file("<" + name + get_propertis_string(properties) + ">", File, Level);

        Level++;

        write_to_file(value, File, Level);

        for_each_node(nodes.begin(), nodes.begin(), NodeFileWriter(File, Level));        

        Level--;

        write_to_file("</" + name + ">", File, Level);
    }

}

std::string Node::ToString(int32_t &Level) const
{
    if(!nodes.size() && value == "")
        return to_string("<" + name + get_propertis_string(properties) + "/>", Level);
    else{
        std::string outData = to_string("<" + name + get_propertis_string(properties) + ">", Level);

        Level++;
        
        outData += to_string(value, Level); 

        for_each_node(nodes.begin(), nodes.end(), NodeStringAppender(&outData, Level));

        Level--;

        outData += to_string("</" + name + ">", Level);

        return outData;
    }
    
}

void Node::ClearNodes()
{
    for_each_node(nodes.begin(), nodes.end(), [](Node * ProcessNode) { delete ProcessNode;} );
    nodes.clear();
}

NodeIterator Node::begin()
{
    return NodeIterator(this, (nodes.size()) ? nodes.begin()->first : "", 0);
}

NodeIterator Node::end()
{
    return NodeIterator(this, "LastFor" + Utils::to_string(this), 0);
}

ConstNodeIterator Node::begin() const
{
    return ConstNodeIterator(this, (nodes.size()) ? nodes.begin()->first : "", 0);
}

ConstNodeIterator Node::end() const
{
    return ConstNodeIterator(this, "LastFor" + Utils::to_string(this), 0);
}

XmlData::XmlData() : 
    stringState(false), 
    shieldChar(false), 
    isSingleNode(false), 
    nodeClosed(false),
    varAsEmptyStr(false),
    lastNode(NULL),
    state(STATE_IDLE), 
    structState(STRUCT_STATE_NODE_DEFINITION), 
    headerState(HEADER_STATE_NOT_SET)  {}

void XmlData::ProcessNodeDefinitionStart() throw (XmlException)
{
    if(state != STATE_IDLE)
        throw cursor.CreateException("Invalid syntax");

    if(text != ""){
        if(!lastNode)
            throw cursor.CreateException("Invalid syntax");

        lastNode->SetValue(lastNode->GetValue() + text);
        text = "";
    }

    lastNode = (processingNodes.size() == 1) ? processingNodes[0] : new Node();

    state = STATE_NODE_NAME;
    structState = STRUCT_STATE_NODE_DEFINITION;
}

void XmlData::Cursor::ProcessChar(char Char)
{
    if(Char == '\n'){
        line++;
        coll = 1;
    }else
        coll++;
}

XmlSyntaxException XmlData::Cursor::CreateException(const std::string &Message)
{
    std::stringstream sstrm;
    sstrm << Message << ", Line " << line << " Coll " << coll;
    return XmlSyntaxException(sstrm.str());
}

void XmlData::ProcessNodeDefinitionEnd() throw (XmlException)
{
    const std::string &nodeName = lastNode->GetName();

    if(nodeName == "")
        throw cursor.CreateException("Empty node name");

    if(varName != ""){
        lastNode->AddProperty(varName, varData);
        varName = varData ="";
    }             

    if(headerState != HEADER_STATE_NOT_SET){
        if(nodeName != "xml")
            throw cursor.CreateException("Invalid header name");

        if(headerState != HEADER_STATE_END)
            throw cursor.CreateException("Invalid header syntax");

        std::vector<std::string> propsNames = lastNode->GetPropertiesNames();

        std::vector<std::string>::const_iterator ci;
        for(ci = propsNames.begin(); ci != propsNames.end(); ++ci){
            const std::string &name = *ci;
            const std::string &val = lastNode->GetProperty(name);
            header.insert(std::make_pair(name, val));
        }

        delete lastNode;

        headerState = HEADER_STATE_NOT_SET;
    }else{

        if(!isSingleNode)
            if(nodeClosed){                        
                if(processingNodes.size() == 1 || processingNodes.back()->GetName() != lastNode->GetName())
                    throw cursor.CreateException("Invalid closing node syntax");
        
                delete lastNode;

                processingNodes.pop_back();                
                lastNode = processingNodes.back();
            }else{                
                if(processingNodes.size() > 1)
                    processingNodes.back()->nodes[nodeName].push_back(lastNode);

                processingNodes.push_back(lastNode);
            }
        else{
            if(processingNodes.back() == lastNode)
                throw cursor.CreateException("Invalid single node");

            processingNodes.back()->nodes[nodeName].push_back(lastNode);
        }
    }

    nodeClosed = isSingleNode = varAsEmptyStr = false;
    structState = STRUCT_STATE_NODE_DATA;
    state = STATE_IDLE;
}

void XmlData::ProcessSlash() throw (XmlException)
{
    if(structState == STRUCT_STATE_NODE_DATA){
        text += '/';        
        return;
    }

    if(state == STATE_IDLE)
        throw cursor.CreateException("Invalid slash syntax");

    isSingleNode = lastNode->GetName() != "";

    nodeClosed = true;
}

void XmlData::ProcessSpace() throw (XmlException)
{
    if(structState == STRUCT_STATE_NODE_DATA){
       text += ' ';
       return;
    }
        
    if(lastNode->GetName() == "")
        state = STATE_NODE_NAME;
    else if(varData != "" || varAsEmptyStr){
        lastNode->AddProperty(varName, varData);
        varName = varData ="";
        state = STATE_NODE_ATTR_NAME;
        varAsEmptyStr = false;
    }else if(state != STATE_IDLE && state != STATE_NODE_ATTR_VAL)
        state = STATE_NODE_ATTR_NAME;
}

void XmlData::ProcessQuestionChar() throw (XmlException)
{
    if(structState == STRUCT_STATE_NODE_DATA){
        text += '?';
        return;
    }

    if(state == STATE_IDLE)
        throw cursor.CreateException("Invalid header syntax");
        
    if(headerState == HEADER_STATE_NOT_SET){
        if(lastNode != processingNodes[0])
            throw cursor.CreateException("Invalid header syntax");

        if(lastNode->GetName() != "")
            throw cursor.CreateException("Invalid header syntax");

        lastNode = new Node();
    }        
            
    headerState = (headerState == HEADER_STATE_BEGIN) ? HEADER_STATE_END : HEADER_STATE_BEGIN;
}

void XmlData::ProcessEqualChar() throw (XmlException)
{
    if(structState == STRUCT_STATE_NODE_DATA){        
        text += '=';
        return;
    }
        
    if(state != STATE_NODE_ATTR_NAME)
        throw cursor.CreateException("Invalid property syntax");

    state = STATE_NODE_ATTR_VAL;
}

void XmlData::ProcessChar(char Char) throw (XmlException)
{
    cursor.ProcessChar(Char);

    if(Char == '"'){
        stringState = !stringState;

        if(!stringState && state == STATE_NODE_ATTR_VAL && varData == "")
            varAsEmptyStr = true;

        return;
    }

    if(Char == '\\'){
        if(!stringState)
            throw cursor.CreateException("Slash out of the string");

        shieldChar = !shieldChar;
        return;
    }

    if(stringState || shieldChar){
        if(state == STATE_IDLE)
            text += Char;
        else if(state == STATE_NODE_ATTR_VAL)
            varData += Char;
        else 
            throw cursor.CreateException("Invalid literal");
    
        if(shieldChar)
            shieldChar = false;
        return;
    }

    if(Char == '<'){
        ProcessNodeDefinitionStart();
    }else if(Char == '>'){
       ProcessNodeDefinitionEnd();
    }else if(Char == '/'){
        ProcessSlash();
    }else if(Char == ' '){
        ProcessSpace();
    }else if(Char == '?'){
        ProcessQuestionChar();
    }else if(Char == '='){
        ProcessEqualChar();
    }else if(!isspace(Char)){
        if(state == STATE_NODE_NAME)
            lastNode->SetName(lastNode->GetName() + Char);
        else if(state == STATE_NODE_ATTR_NAME)
            varName += Char;
        else if(state == STATE_NODE_ATTR_VAL)
            varData += Char;
        else
            text += Char;
    }
}

void XmlData::Cleanup()
{
    state = STATE_IDLE;
    structState = STRUCT_STATE_NODE_DEFINITION;

    stringState = shieldChar = isSingleNode = nodeClosed = varAsEmptyStr = false;

    text = varName = varData = "";

    processingNodes.clear();

    lastNode = NULL;

    cursor.Clear();
}

void XmlData::LoadFromFile(const std::string &FilePath) throw (XmlException)
{
    Utils::FileGuard file(FilePath, "r");

	Utils::AutoEvent evnt(std::bind(&XmlData::Cleanup, this));

    const size_t blockSize = 1024;
    char block[blockSize];

    rootNode.ClearNodes();
    processingNodes.push_back(&rootNode);

    while(!feof(file.get())){
        size_t nRead = fread(block, 1, blockSize, file.get());
        for(size_t i = 0; i < nRead; i++)
            ProcessChar(block[i]);
    }
}

void XmlData::LoadFromString(const std::string &DataString) throw (XmlException)
{
	Utils::AutoEvent evnt(std::bind(&XmlData::Cleanup, this));

    rootNode.ClearNodes();
    processingNodes.push_back(&rootNode);

    std::string::const_iterator ci;
    for(ci = DataString.begin(); ci != DataString.end(); ++ci)
        ProcessChar(*ci);
}

std::string XmlData::ToString() const
{
    std::string outString = to_string(header_to_string(header), 0);

    int32_t level = 0;
    return outString + rootNode.ToString(level);
}

void XmlData::SaveToFile(const std::string &FilePath) const throw (XmlException)
{
    Utils::FileGuard file(FilePath, "w");

    write_to_file(header_to_string(header), file.get(), 0);

    int32_t level = 0;
    rootNode.WriteToFile(file.get(), level);
}
}
