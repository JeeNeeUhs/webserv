#ifndef EXECUTE_HPP
# define EXECUTE_HPP

# include <string>
# include <vector>
# include "Config/Config.hpp"

class Execute {
	private:
		std::vector<Server> servers;
	
	public:
		Execute();
		Execute(const Execute& other);
		Execute& operator=(const Execute& other);
		~Execute();

		

};

#endif
