import os, random, glob, fnmatch, time
import subprocess, multiprocessing
from dotenv import load_dotenv

load_dotenv()

class ParalelSimulation():
    def __init__(self, project_path, directory_path):
        self.project_path = project_path
        self.directory_path = directory_path

    def openx_files(self):
        xosc_files = fnmatch.filter(os.listdir(self.directory_path), '*.xosc')
        abbreviated_path = [os.path.normpath(path) for path in [os.path.join(self.directory_path, file) for file in xosc_files]]
        fixed_path = [path.replace('\\', '/') for path in abbreviated_path]
        
        if self.directory_path == os.environ.get('OPENX_SAMPLE_PATH'):
            return random.sample(fixed_path, 10)
        elif self.directory_path == os.environ.get('OPENX_FOLDER_PATH'):
            return fixed_path
        else:
            return None

    def run_test_cases(self, test_case):
        command = f"{self.project_path}/bin/esmini --window 60 60 800 400 --osc ./{test_case}"
        subprocess.call(command, shell=True)
    
    def paralel_processing(self, test_cases):
        pool = multiprocessing.Pool()
        pool.map(self.run_test_cases, test_cases)
        pool.close()
        pool.join()

if __name__ == "__main__":
    start = time.time()
    
    project_path = os.path.dirname(os.path.join(os.getcwd(), os.listdir(os.getcwd())[0]))
    directory_path = os.environ.get('OPENX_SAMPLE_PATH')
    # directory_path = os.environ.get('OPENX_FOLDER_PATH')

    simulation = ParalelSimulation(project_path, directory_path)
    xosc = simulation.openx_files()
    simulation.paralel_processing(xosc)
    
    end = time.time()
    print(f"\nExecution Time: {end-start}")