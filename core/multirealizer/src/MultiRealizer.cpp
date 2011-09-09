
#include "vhcl.h"

#if defined(WIN_BUILD)
#include <windows.h>
#include <mmsystem.h>
#include <conio.h>
#endif

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <map>
#include <set>
#include <sstream>

using std::string;
using std::map;

#include "vhmsg-tt.h"
#include "vhmsg.h"

#define TEST_WAIT  0

int numMessagesReceived = 0;
class Dialogue
{
	public:
		Dialogue()
		{
		}

		~Dialogue()
		{
			_lines.clear();
		}

		int getNumLines()
		{
			return _lines.size();
		}

		const std::string& getLine(int num, std::string& role)
		{
			if (num < 0 || num >= (int) _lines.size())
				return _emptyLine;

			role = _lines[num].first;
			return _lines[num].second;
		}

	protected:
		std::vector<std::pair<std::string, std::string> > _lines;
		std::string _emptyLine;
	
};

class CrabCanonDialogue : public Dialogue
{
	public:
		CrabCanonDialogue() : Dialogue()
		{
			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "Good day, Mr. A."));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", "Why, same to you"));
			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "So nice to run into you."));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", "That echoes my thoughts."));
			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "And it's a perfect day for a walk. I think I'll be walking home soon."));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", "Oh, really? I guess there's nothing better for you than walking."));
			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "Incidentally, you're looking in fine fettle these days, I must say."));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", "Thank you very much."));
			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "Not at all. Here, care for one of my cigars?"));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", "Oh, you are such a philistine. In this area, the Dutch contributions are of markedly inferior taste, don't you think?"));
			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "I disagree, in this case. But speaking of taste, I finally saw that \
    Crab Canon by your favorite artist, M.C. Escher, in a gallery the other \
    day, and I fully appreciate the beauty and ingenuity with which he \
    made one single theme mesh with itself going both backwards and \
    forwards. But I am afraid I will always feel Bach is superior to Escher."));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", "I don't know. But one thing for certain is that I don't worry about arguments of taste. De gustibus non est disputandum."));
			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "Tell me, what's it like to be your age? Is it true that one has no worries at all?"));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", " To be precise one has no frets."));
			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "Oh, well, it's all the same to me."));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", "Fiddle. It makes a big difference, you know."));
			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "Say, don't you play the guitar?"));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", "That's my good friend. He often plays, the fool. But I myself wouldn't touch a guitar with a ten-foot pole."));
			_lines.push_back(std::pair<std::string, std::string>("Crab", " Hallo! Hullo! What's up? What's new? You see this bump, this \
    from Warsaw - a collosal bear of a man -  playing a lute. He was three \
    meters tall, if I'm a day. I mosey on up to the chap, reach skyward and \
    manage to tap him on the knee, saying, \"Pardon me, sir, but you are \
    Pole-luting our park with your mazurkas.\" But WOW! he had no sense \
    of humor - not a bit, not a wit - and POW! - he lets loose and belts me \
    one, smack in the eye! Were it in my nature, I would crab up a storm, \
    but in the time-honored tradition of my species, I backed off. After all, \
    when we walk forwards, we move backwards. It's in our genes, you \
    know, turning round and round. That reminds me - I've always \
    wondered, \"which came first - the Crab or the Gene?\" That \
    is to say, \"Which came last - the Gene, or the Crab?\" I'm always \
    turning things round and round, you know. It's in our genes, after \
    all. When we walk backwards we move forwards. Ah me, oh my! \
    I must lope along on my merry way - so off I go on such a fine day. \
    Sing \"ho!\" for the life of a Crab! TATA! Ole!"));

			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "That's my good friend. He often plays, the fool. But I myself wouldn't touch a ten-foot Pole with a guitar."));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", "Say, don't you play the guitar?"));
			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "Fiddle. It makes a big difference, you know."));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", "Oh, well, it's all the same to me."));
			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "To be precise one has no frets."));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", "Tell me, what's it like to be your age? Is it true that one has no worries at all?"));
			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "I don't know. But one thing for certain is that I don't worry about arguments of taste. Disputandum non est de gustibus."));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", "I disagree, in this case. But speaking of taste, I finally heard that \
    Crab Canon by your favorite composer, J.S. Bach, in a concert the \
    other day, and I fully appreciate the beauty and ingenuity with which \
    he made one single theme mesh with itself going both backwards and \
    forwards. But I am afraid I will always feel Escher is superior to Bach."));
			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "Oh, you are such a philistine. In this area, the Dutch contributions are of markedly inferior taste, don't you think?"));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", "Not at all. Here, care for one of my cigars?"));
			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "Thank you very much."));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", "Incidentally, you're looking in fine fettle these days, I must say."));
			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "Oh, really? I guess there's nothing better for you than walking."));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", "And it's a perfect day for a walk. I think I'll be walking home soon."));
			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "That echoes my thoughts."));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", "So nice to run into you."));
			_lines.push_back(std::pair<std::string, std::string>("Tortoise", "Why, same to you"));
			_lines.push_back(std::pair<std::string, std::string>("Achilles", "Good day, Mr. A."));
		}
};

class WorldState
{
public:
	WorldState()
	{
		_dialogue = NULL;
	}

	void setDialogue(Dialogue* dialogue)
	{
		_dialogue = dialogue;
	}

	Dialogue* getDialogue()
	{
		return _dialogue;
	}

	Dialogue* _dialogue;
	std::set<std::string> characters;
	std::string role;
	std::string curCharacter;

};



WorldState world;

void tt_client_callback( const char * op, const char * args, void * user_data )
{
	printf( "received - '%s %s'\n", op, args );


	std::string message = op;
	std::string messageArgs = args;

	if (message == "wsp")
	{
		std::vector<std::string> tokens;
		vhcl::Tokenize(messageArgs, tokens, " ");
		if (tokens.size() < 1)
			return;
		std::string wspMessge = tokens[0];
		if (wspMessge == "answer-attribute-value")
		{
			if (tokens.size() < 2)
				return;
		
			if (tokens.size() < 6)
				return;
			std::string symbol = tokens[2];
			if (tokens[3] == "position")
			{
				if (tokens[4] != "update")
					return;
				std::vector<std::string> positions;
				vhcl::Tokenize(tokens[5], positions, ",");
				if (positions.size() < 3)
					return;
				double x = atof(positions[0].c_str());
				double y = atof(positions[1].c_str());
				double z = atof(positions[2].c_str());

				// determine if this symbol exists
				std::set<std::string>::iterator iter = world.characters.find(symbol);
				if (iter == world.characters.end())
				{
					std::stringstream strstr;
					strstr <<  "pawn " << symbol << " init loc 0 0 0";
					vhmsg::ttu_notify2( "sbm", strstr.str().c_str());
				}
				// update the pawn's position
				std::stringstream strstr;
				strstr << "set pawn " << symbol << " world_offset x " << x << " y " << (y + 200) << " z " << z;
				vhmsg::ttu_notify2( "sbm", strstr.str().c_str());
			}

		}
		
	}
	else if (message == "<blockstart")
	{
		// do nothing for now
		return;
	}
	else if (message == "<blockend")
	{
		std::vector<std::string> tokens;
		vhcl::Tokenize(messageArgs, tokens, " ");
		std::string characterName;
		std::string id;
		size_t pos;
		for (size_t t = 0; t < tokens.size(); t++)
		{
			pos = tokens[t].find("characterId");
			if (pos != std::string::npos)
			{
				// get the character name
				std::string characterNameTemp = tokens[t].substr(pos + 13);
				int pos2 = characterNameTemp.find_first_of("\"");
				if (pos2 != std::string::npos)
				{
					characterName = characterNameTemp.substr(0, pos2);
				}
				else
				{
					characterName = characterNameTemp;
				}
				continue;
			}

			pos = tokens[t].find("id");
			if (pos != std::string::npos)
			{
				std::string idTemp = tokens[t].substr(pos + 4);
				int pos2 = idTemp.find("\"");
				if (pos2 != std::string::npos)
				{
					id = idTemp.substr(0, pos2);
				}
				else
				{
					id = idTemp;
				}
				// determine which dialogue line is being spoken
				std::string sentenceStr;
				int pos3 = id.find("sentence_");
				if (pos3 != std::string::npos)
				{
					sentenceStr = id.substr(pos3 + 9);
					int lineNum = atoi(sentenceStr.c_str());
					// determine who is speaking that line
					Dialogue* dialogue = world.getDialogue();
					if (dialogue && (lineNum > 0) && (dialogue->getNumLines() > lineNum) )
					{
						std::string role;
						int nextLine = lineNum + 1;
						if (nextLine == dialogue->getNumLines())
							nextLine = 0;
						const std::string& utterance = dialogue->getLine(nextLine, role); 
						if (role == world.role)
						{
							std::cout << "Uttering next line of dialogue..." << std::endl;
							std::stringstream strstr;
							/*
							strstr << "bml char " << world.curCharacter << " speech " << utterance;
							vhmsg::ttu_notify2( "sbm", strstr.str().c_str());
							*/
							// until SmartBody is working, just relay the blockstart/blockend messages
							strstr << "<blockstart characterId=\"" << world.curCharacter << "\" id=\"sentence_" << nextLine << "\"/>";
							vhmsg::ttu_notify2(strstr.str().c_str(), "");
							strstr << "<blockend characterId=\"" << world.curCharacter << "\" id=\"sentence_" << nextLine << "\"/>";
							vhmsg::ttu_notify2(strstr.str().c_str(), "");
						}
					}
				}
				continue;
			}
		}
	}
}

int main( int argc, char * argv[] )
{
   int err;

	vhmsg::ttu_set_client_callback( tt_client_callback );

   err = vhmsg::ttu_open();
   if ( err == vhmsg::TTU_SUCCESS )
   {
      printf( "VHMSG_SERVER: %s\n", vhmsg::ttu_get_server() );
      printf( "VHMSG_PORT: %s\n", vhmsg::ttu_get_port() );
      printf( "VHMSG_SCOPE: %s\n", vhmsg::ttu_get_scope() );

	  err = vhmsg::ttu_register( "*" );

	  vhmsg::ttu_report_version( "multirealizer", "all", "all" );
   }
   else
   {
      printf( "Connection error!\n" );
      return -1;
   }

   if (argc > 1)
   {
	   world.curCharacter = argv[1];
   }
   else
   {
	   world.curCharacter = "brad";
   }

   if (argc > 2)
   {
	   world.role = argv[2];
   }
   else
   {
	   world.role = "Tortoise";
   }

   CrabCanonDialogue dialogue;
   world.setDialogue(&dialogue);


   const int NUM_MESSAGES = 20000;

   while ( 1) //!vhcl::kbhit() )
	{
		err = vhmsg::ttu_poll();
		if( err == vhmsg::TTU_ERROR )
		{
		   printf( "ttu_poll ERR\n" );
		}
	}
    
//            vhmsg::ttu_notify2( "elbench", s );


   vhmsg::ttu_close();

   return 0;
}
