#include <iostream>
#include <vector>
#include <memory>
#include <stack>
#include <functional>
#include <algorithm>
using namespace std;

struct XmlTag {
  //enum class Tag { Screen, Name, Msg, XTable };
  string name;
  string value;
  std::vector<XmlTag> child;
  std::vector<std::pair<string, string>> attributes;

  XmlTag(const string& tag, const string& value = "") : name { tag }, value { value } {}
  XmlTag(const string& tag, const std::vector<XmlTag>& child) : name { tag }, child { child } {}
  //~XmlTag() { cout << "~XmlTag" << endl; }

  void print(const XmlTag& root) const;
};
std::vector<XmlTag> fillVector();

struct Controller;
struct View {
  std::weak_ptr<Controller> controller_;

  void setController(std::weak_ptr<Controller> controller)
  {
    controller_ = controller;
  }

  void display(std::vector<string> options);

  ~View() { cout << "~View" << endl; }
};

struct Controller {
  std::weak_ptr<View> view_;
  std::stack<std::reference_wrapper<XmlTag>> displayStack_;
  std::vector<XmlTag> tags_;

  ~Controller() { cout << "~Controller" << endl; }

  void setView(std::weak_ptr<View> view)
  {
    view_ = view;
  }

  void onEntry()
  {
    tags_ = fillVector();
    displayStack_.push(tags_.front());
    std::vector<string> options;

    auto tag = displayStack_.top().get().child;
    for (auto elem : tag)
      if (elem.name == "Name")
	options.emplace_back(elem.value);

    auto sp = view_.lock();
    if (sp)
      sp->display(options);
  }

  void keyPressed(int opt)
  {
    switch(opt)
    {
      case 2:
	backPressed();
	break;
      default:
	optionSelected(opt);
    }
  }

  void backPressed()
  {
    displayStack_.pop();
    sendOption();
  }

  void optionSelected(int opt)
  {
    auto child = displayStack_.top().get().child[opt-1];
    child.print(child);
    auto it = std::find_if(std::begin(tags_), std::end(tags_), 
	[&](const XmlTag& tag) { return child.value == tag.name; });
    if (it != tags_.end())
    {
      displayStack_.push(*it);
      sendOption();
    }
    else
    {
      std::cout << "element not found" << std::endl;
    }
  }

  void sendOption()
  {
    std::vector<string> options;
    auto tag = displayStack_.top().get().child;
    for (auto elem : tag)
      if (elem.name == "Name")
	options.emplace_back(elem.value);

    auto sp = view_.lock();
    if (sp)
      sp->display(options);
  }
};

struct Manager {
  std::shared_ptr<View> view;
  std::shared_ptr<Controller> controller;

  Manager() {
    view = std::make_shared<View>();
    controller = std::make_shared<Controller>();
    view->setController(controller);
    controller->setView(view);
  }
  ~Manager() { cout << "~Manager" << endl; }

  void onEntry()
  {
    controller->onEntry();
  }
};

int main()
{
  Manager manager;
  manager.onEntry();
}

std::vector<XmlTag> fillVector()
{
  XmlTag root("Auto");

  XmlTag tag1 ("Screen1");
  tag1.child.emplace_back("Name", "Title: Title 1");
  tag1.child.back().attributes.emplace_back("Category", "00");

  tag1.child.emplace_back("Name", "Back");
  tag1.child.back().attributes.emplace_back("Category", "01");

  tag1.child.emplace_back("Name", "Screen2");
  tag1.child.back().attributes.emplace_back("XTable", "Screen2");

  tag1.child.emplace_back("Name", "Screen3");
  tag1.child.back().attributes.emplace_back("Category", "03");
  tag1.child.back().attributes.emplace_back("XTable", "Screen3");

  XmlTag tag2 ("Screen2");
  tag2.child.emplace_back("Name", "Title: Title 2");
  tag2.child.back().attributes.emplace_back("Category", "00");

  tag2.child.emplace_back("Name", "Back");
  tag2.child.back().attributes.emplace_back("Category", "01");

  tag2.child.emplace_back("Name", "Screen3 Option");
  tag2.child.back().attributes.emplace_back("Category", "02");
  tag2.child.back().attributes.emplace_back("XTable", "Screen3");

  tag2.child.emplace_back("Name", "Screen3");
  tag2.child.back().attributes.emplace_back("Category", "03");
  tag2.child.back().attributes.emplace_back("XTable", "Screen3");

  XmlTag tag3 ("Screen3");
  tag3.child.emplace_back("Name", "Title: Title 3");
  tag3.child.back().attributes.emplace_back("Category", "00");

  tag3.child.emplace_back("Name", "Back");
  tag3.child.back().attributes.emplace_back("Category", "01");

  tag3.child.emplace_back("Name", "Failure");
  tag3.child.back().attributes.emplace_back("Category", "02");

  tag3.child.emplace_back("Name", "Errors");
  tag3.child.back().attributes.emplace_back("Category", "03");

  root.child.emplace_back(tag1);
  root.child.emplace_back(tag2);
  root.child.emplace_back(tag3);

  //root.print(root);

  return {tag1, tag2, tag3};
}

void XmlTag::print(const XmlTag& root) const
{
  std::cout << "<" << root.name;
  for (auto& attrib : root.attributes)
    std::cout << " " << attrib.first << "=" << attrib.second;

  if (root.value.empty())
    std::cout << ">" << std::endl;
  else
    std::cout << ">" << root.value;

  for (auto& child : root.child)
    child.print(child);
  std::cout << "</" << root.name << ">" << std::endl;
}

void View::display(std::vector<string> options)
{
  int x = 1;

  for (auto& opt : options)
    std::cout << x++ << ": " << opt << std::endl;

  std::cout << "Please choose the option: ";
  std::cin >> x;

  auto sp = controller_.lock();
  if (sp)
  {
    sp->keyPressed(x);
  }
}
