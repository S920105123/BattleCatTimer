#!/opt/local/bin/python

# This script compares two path reports, producing a score of the test report against a reference.
# This is for the TAU 2018 contest.
#
# As you can see, these are very strict assumptions, so do not try to
# use this script for anything other than the TAU benchamrks
#

import argparse
import sys
import re

# global patterns
START_PATH_RE = re.compile("^Endpoint")
END_PATH_RE = re.compile("^\s*$")
NODE_LINE_RE = re.compile("\s+(v|\^)\s+(\S+)")
ac = 1

class paths:
    """ store all path """

    def __init__(self, name):
        self.name_ = name
        self.allPaths_ = { }
        self.path_list = []
        self.index_ = 0
        self.num = 0

    def __len__(self):
        return self.num
        # return len(self.allPaths_)

    def appendPath(self, lines):

        pathKey = ""
        # we create a key which is just all the nodes concatenated together with '$'
        for line in lines:
            matchObj = re.search(NODE_LINE_RE, line)
            if matchObj:
                pathKey = pathKey + "$" + matchObj.group(2)
                
            
        #print "Storing path " + pathKey + " with index " + str(self.index_)
        self.allPaths_[pathKey] = self.index_
        self.index_ = self.index_ + 1
        self.path_list.append( pathKey )
        self.num = self.num + 1

    def getPathCount(self):
        return self.index_

    def compare(self, reference_paths):

        score = 0
        
        global ac
        for i in range(0, self.index_):
            if i < len(reference_paths.path_list) and self.path_list[i] != reference_paths.path_list[i]:
                print  "WA: {1} is no find{0}".format(self.path_list[i], i)
                ac = 0
                return score

        return score

        # compare self with a reference set of paths
        # id = 0
        # for path in self.allPaths_:
            # id = id + 1
            # if path in reference_paths.allPaths_:
                # reference_index = reference_paths.allPaths_[path]
                # test_index = self.allPaths_[path]
                # # print "id = {0} find".format(id)
                # if test_index != reference_index:
                    # print "test_index = {0}; reference_index = {1} {2}".format(test_index, reference_index, path)
                    # score = score + abs(reference_index - test_index)
            # else:
                # # print "Error: Cannot find path " + path + "in reference"
                # # print "Adding score of {0}".format(self.getPathCount())
                # print "id = {0} not find {1}".format(id, path)
                # score = score + self.getPathCount()

        # return score

# read_report -- read the path report and store individual paths
def read_report(filename):

    # print "Reading path report " + filename

    # dictionary to be returned
    my_paths = paths(filename)
    
    # store whole file in memory
    all_lines = [ ]
    with open(filename) as f:
        all_lines = f.readlines()

    #
    # possible states
    #    search_path = searching for beginning of path (BEG_PATH_RE)
    #    capture_path = adding lines until we see end of path
    #
    state = "search_path"
    path_lines = [ ]
    for line in all_lines:
        if state == "search_path":
            
            # check for cell
            matchObj = re.match(START_PATH_RE, line)
            if matchObj:
                state = "capture_path"
                path_lines.append(line)
                continue

            # not a match, so keep searching
            continue

        if state == "capture_path":

            path_lines.append(line)

            # check for closing
            matchObj = re.match(END_PATH_RE, line)
            if matchObj:

                my_paths.appendPath(path_lines)

                path_lines = [ ]
                state = "search_path"
                continue

    return my_paths

def main():

    global ac
    parser = argparse.ArgumentParser()
    parser.add_argument('--reference', required=True)
    parser.add_argument('--test', required=True)

    args = parser.parse_args()

    if not args.reference or not args.test:
        parser.print_help()

    # print "Executing {0} with options reference='{1}' and test='{2}'".format(sys.argv[0],args.reference, args.test)


    # read referece report
    reference_paths = read_report(args.reference)
 
    # read test report
    test_paths = read_report(args.test)

    # compare and print results to stdout
    # print "\n###\nComparing Test against Reference\n###"
    # print "Comparing Test against Reference"

    print ' # of paths: ', len(reference_paths), ' and ' , len(test_paths)
    ac = 1
    score1 = test_paths.compare(reference_paths)
    # print "Score = {0}".format(score1)

    # print "\n###\nComparing Reference against Test\n###"
    score2 = reference_paths.compare(test_paths)
    # print "Score = {0}".format(score2)
    # print "\n\n###\nTotal Score = {0}\n###\n".format(score1 + score2)

    if len(reference_paths) != len(test_paths):
        print "WA!"
        ac = 0

    if ac == 1:
        print "AC!"
    exit(0);


if __name__ == "__main__":
    main()
