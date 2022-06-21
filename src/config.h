#ifndef TRT_CONFIG_H
#define TRT_CONFIG_H

#include <map>
#include <string>

namespace trt {

	/* Config stores arguments to trt as a map: "arg string => parameter string"
	 * Arguments without a parameter are stored as empty strings.
	 * Arguments may not begin with a digit, as this could be confused with a negative parameter.
	 * E.g. "trt -n 5 -p -q 'a b'" yields ['n' => 5, 'p' => '', 'q' => 'a b'];
	 * Repeated arguments supersede old ones.
	 * Boolean interpretation: "1" is true, "0" false, invalid otherwise */
	class Config : public std::map<std::string, std::string>{
	  public:
		  // Load from arguments to main
		  void loadArgs(int argc, char** argv);
		  /* Load from string in format:
		   * argument<whitespace>[optional parameter]<newline>, e.g.:
		   * "n 5\np\nq a b\n" (gives same result as above) */
		  //void loadFile(std::string filename);

	      // Indicates whether an argument has been set at all.
		  //bool			isset(std::string key);
		  /* Functions to parse parameters into string, int, double, bool.
		   * These throw invalid_argument if no conversion could be performed,
		   * out_of_range if the value is out of range */
		  //std::string	getString(std::string key);
		  //int			getInt(std::string key);
		  //double		getDouble(std::string key);

	      // Returns true if key="1", false if key="0", invalid_argument otherwise.
		  //bool			getBool(std::string key);
	};
}

#endif
