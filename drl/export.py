import network
import numpy as np
import torch
import matplotlib.pyplot as plt
import pickle


def export(load_state_path, model_save_path):
    model = network.Model()
    model.load_state_dict(torch.load(load_state_path))

    tmp_input = torch.rand(1, 3, 15, 15)

    traced_script_model = torch.jit.trace(model, tmp_input)

    output_value, output_prob = model(tmp_input)
    output_value_, output_prob_ = traced_script_model(tmp_input)

    n = np.ones((1, 3, 15, 15), dtype=np.float32)
    i = torch.from_numpy(n)
    print(traced_script_model(i))

    np.testing.assert_allclose(output_value.detach().numpy(), output_value_.detach().numpy())
    np.testing.assert_allclose(output_prob.detach().numpy(), output_prob_.detach().numpy())

    traced_script_model.save(model_save_path)


def export_loss(path):
    file = open(path, 'rb')
    record = np.array(pickle.load(file))
    file.close()

    plt.plot(record[:, 1])
    plt.show()


export_loss('loss_record_resnet.pkl')
# export('state/state_500.pkl', '../model/model_500.pt')
