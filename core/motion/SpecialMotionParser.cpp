#include <common/RobotInfo.h>
#include <motion/SpecialMotionParser.h>

int jointMapping[NUM_JOINTS] = {
  HeadYaw,
  HeadPitch,

  LShoulderPitch,
  LShoulderRoll,
  LElbowYaw,
  LElbowRoll,

  RShoulderPitch,
  RShoulderRoll,
  RElbowYaw,
  RElbowRoll,

  LHipYawPitch,
  LHipRoll,
  LHipPitch,
  LKneePitch,
  LAnklePitch,
  LAnkleRoll,

  RHipYawPitch,
  RHipRoll,
  RHipPitch,
  RKneePitch,
  RAnklePitch,
  RAnkleRoll
};

string jointName1[NUM_JOINTS]= {
  "HY", "HP"  "LSP" "LSR" "LEY" "LER"  "RSP" "RSR" "REY" "RER"  "LHYP" "LHR" "LHP"  "LKP"  "LAP" "LAR"  "RHYP" "RHR" "RHP"  "RKP"  "RAP" "RAR"};


float startJoints[NUM_JOINTS] = {0,30,-1.8,-26,71,-45,1.8,0,-1.8,-26,71,-45,1.8,-90,30,0,0,-90,30,0,0,0};

float endJoints[NUM_JOINTS] = {0, 0, 0, 0, 0, 11.46, -10.32, 0, 0, 0, 0, 11.46, -10.32, 0, -107.08, 16.3, -91.72, -33.45, -107.08, 16.3, -91.72, -33.45};    

SpecialMotion::SpecialMotion():
  mType(Motions::Null),
  time(0.0),
  stiffness(1.0)
{
  for(int i=0;i<NUM_JOINTS;i++)
  {
    jointMotions[i]=VOID_NUM;
  }     
};
SpecialMotion::SpecialMotion(int type):
  mType(Motions::JointAngle),
  time(0.0),
  stiffness(1.0)
{
  if(type==Motions::null)
  {
    for(int i=0;i<NUM_JOINTS;i++)
    {
      jointMotions[i]=VOID_NUM;
    } 
  }
  else if (type==Motions::start)
  {
    time=200.0;
    for(int i=0;i<NUM_JOINTS;i++)
      jointMotions[i] = startJoints[i];
    //jointMotions={0,30,-1.8,-26,71,-45,1.8,	0,-1.8,-26,71,-45,1.8,-90,30,0,0,-90,30,0,0,0};

  }
  else if (type==Motions::end)
  {
    time=400.0; 
    //  jointMotions= {0, 0, 0, 0, 0, 0.2, -0.18, 0, 0, 0, 0, 0.2, -0.18, 0, 1.868, 0.284, -1.6, 0.584, 1.868, 0.284, -1.6, 0.584}; 
    for(int i=0;i<NUM_JOINTS;i++)
      jointMotions[i] = endJoints[i];
    //jointMotions= {0, 0, 0, 0, 0, 11.46, -10.32, 0, 0, 0, 0, 11.46, -10.32, 0,
    //-107.08, 16.3, -91.72, -33.45, -107.08, 16.3, -91.72, -33.45};    
  }
};


string SpecialMotionParser::getMotionIdName(string& s)
{
  string nameMotion="";
  size_t found=s.find("motion_id");
  if(found!=string::npos) 
  {
    string tmps=s.substr(int(found),s.length()-1);
    size_t foundName=tmps.find_first_not_of("= ");
    if(foundName!=string::npos)
      nameMotion=tmps.substr((int)foundName,tmps.length()-1); 
  }
  std::cout<<"Name = "<<nameMotion<<std::endl;
  return nameMotion;
}


bool SpecialMotionParser::ParseMotionFile(const string& mofFilePath,vector<SpecialMotion>& MotionSequence)
{
  string nameMotion;

  bool findName=false;
  bool labelStart=true;
  bool reMapping=false;

  vector<int> fileMapping;
  int mappingSize=NUM_JOINTS;

  string line;  
  ifstream infile;
  infile.open(mofFilePath.c_str());
  int l=0; // to record the line number;
  if(!infile.is_open())
  {
    std::cout<<"Not Open file"<<std::endl;
    return false;
  }

  while(getline(infile,line))
  {
    //std::cout<<l<<"Line "<<MotionSequence.size()<<": "<<line<<std::endl;
    l++;
    if(line.length()==0) {
      continue;
    } else if(line[0]=='\"'||isspace(line[0])) {
      continue;
    }
    vector<string> tokens;
    ///start tokenlize
    /**

      int start=0; int end=0;

      while(end<line.length())
      {
      if(isspace(line[end]))
      {   
      tokens.push_back(line.substr(start,end-1)); 
      start=(int)line.find_first_not_of(" \t\n",end);
      end=start;
      }
      end++;    
      }
      tokens.push_back(line.substr(start,end-1));
    ///end tokenlize
    */
    size_t start = 0, end = 0;
    char sep=' ';
    while ((end = line.find_first_of(" \t\n",start)) != string::npos) {
      if ((end-start > 0) && (line[start] != ' ')) {
        std::string tok = line.substr(start, end - start);
        if (tok.length() > 0)
          tokens.push_back(tok);
      }
      start = end + 1;
    }
    std::string tok = line.substr(start);
    if (tok.length() > 0)
      tokens.push_back(tok);


    if(!findName) {
      //  nameMotion= getMotionIdName(line);
      findName=true;
      continue;
    } else if(!labelStart&&line.length()>5&&line.substr(0,4).compare("label")==0) {
      //TODO: label start
      continue;
    } else if(line.length()>10&&line.substr(0,10).compare("transition")==0) {
      //TODO: transition: recursively??
    } else if(line.length()>8&&line.substr(0,8).compare("hardness")==0) {
      //set hardness
      SpecialMotion sMotion;
      sMotion.mType=Motions::Hardness; 
      sMotion.stiffness=1.0; 	
      if(tokens.size()!=(mappingSize+2)) {
        cout<<"Wrong line: "<<l<< " (hardness wrong):"<< line<<endl;
        continue;
      }
      for(int i=1; i< tokens.size()-1;i++)  //skip first one
      {
        int index=jointMapping[(reMapping)?fileMapping[i-1]:(i-1)];
        if(tokens[i].compare("*")!=0&&tokens[i].compare("-")!=0)
          sMotion.jointMotions[index]=atof(tokens[i].c_str());
        else
          sMotion.jointMotions[index]=VOID_NUM;
      }
      if(tokens[tokens.size()-1].compare("*")==0||tokens[tokens.size()-1].compare("-")==0)
        sMotion.time=0.0;
      else
        sMotion.time= atof(tokens[tokens.size()-1].c_str());	

      MotionSequence.push_back(sMotion);
    } else if(isdigit(tokens[0][0])||(tokens[0][0]=='-')||(tokens[0][0]=='*')) {
      //set jointMotion
      SpecialMotion sMotion;
      sMotion.mType=Motions::JointAngle; 
      if(tokens.size()!=(mappingSize+2)) {
        cout<<l<<mappingSize<<"Wrong line: "<<tokens.size()<< " (jointAngle wrong):"<< line<<endl;
        continue;
      }
      for(int i=0; i< tokens.size()-2;i++)    {
        int index=jointMapping[(reMapping)?fileMapping[i]:i];
        if(tokens[i].compare("*")!=0&&tokens[i].compare("-")!=0)
          sMotion.jointMotions[index]=atof(tokens[i].c_str());
        else
          sMotion.jointMotions[index]=VOID_NUM;
      }
      sMotion.stiffness=atof(tokens[tokens.size()-2].c_str());
      sMotion.time= atof(tokens[tokens.size()-1].c_str());	
      MotionSequence.push_back(sMotion);
    } else if(findName&&!reMapping&&tokens[0][0]=='H') {

      //TODO:rebuild the mapping, other name??
      for(int i=1; i<tokens.size()-2;i++)
      {
        for(int j=0;j< NUM_JOINTS; j++)
        {	
          if(tokens[i].compare(jointName1[j])==0)   
          {
            fileMapping.push_back(j);	
          }
        }
      }

      mappingSize=fileMapping.size();   //resize
      reMapping=true;
      continue;   
    } else {
      // else:  type not known 
      std::string ignoreLine = "label start";
      if (line.substr(0,ignoreLine.size()) != ignoreLine)
        cout<<"Unknown line: "<<l<< ": "<< line<<endl;
      continue;
    }

  }
  infile.close();
  return true;

  //sequenceMotion(i,m_MotionSequence);

}

